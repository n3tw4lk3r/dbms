#include "catalog/table.hpp"

#include <fstream>

#include "exceptions/database_error.hpp"
#include "storage/indexed_value.hpp"
#include "storage/serializer.hpp"

namespace dbms {

Table::Table(
    const std::string& name,
    const std::vector<ColumnSchema>& schema,
    const std::filesystem::path& storage_path
) :
    name(name),
    schema(schema),
    storage_path(storage_path),
    metadata_path(storage_path.string() + ".meta"),
    data_path(storage_path.string() + ".data")
{
    for (const auto& column : schema) {
        if (column.indexed) {
            indexes.emplace(column.name, BTree());
        }
    }
}

Table::Table(const std::filesystem::path& storage_path) :
    storage_path(storage_path)
{
    load();
}

const std::string Table::getName() const {
    return name;
}

RowId Table::insertRow(const std::vector<Value>& values) {
    validateRow(values);

    auto row = std::make_unique<Row>();

    row->id = next_row_id;
    ++next_row_id;

    row->values = values;

    Row* row_ptr = row.get();

    rows.push_back(std::move(row));
    row_lookup[row_ptr->id] = row_ptr;

    insertIntoIndexes(*row_ptr);
    appendInsert(*row_ptr);

    tryCompact();
    return row_ptr->id;
}

const std::vector<std::unique_ptr<Row>>& Table::getRows() const {
    return rows;
}

std::vector<std::unique_ptr<Row>>& Table::getRowsMutable() {
    return rows;
}

const std::vector<ColumnSchema>& Table::getSchema() const {
    return schema;
}

bool Table::hasIndexedColumn(
    const std::string& column_name
) const {
    return indexes.find(column_name) != indexes.end();
}

bool Table::containsIndexedValue(
    const std::string& column_name,
    const Value& value
) const {
    auto it = indexes.find(column_name);

    if (it == indexes.end()) {
        return false;
    }

    return it->second.contains(
        IndexedValue(value)
    );
}

Row* Table::findByIndexedValue(
    const std::string& column_name,
    const Value& value
) {
    auto it = indexes.find(column_name);

    if (it == indexes.end()) {
        return nullptr;
    }

    RowId row_id = it->second.find(IndexedValue(value));

    if (row_id == 0) {
        return nullptr;
    }

    Row* row = findRowById(row_id);

    if (!row || row->deleted) {
        return nullptr;
    }

    return row;
}

Row* Table::findRowById(RowId row_id) {
    auto it = row_lookup.find(row_id);

    if (it == row_lookup.end()) {
        return nullptr;
    }

    return it->second;
}

void Table::updateRow(
    Row& row,
    const std::vector<Assignment>& assignments
) {
    std::vector<Value> updated_values = row.values;

    for (const auto& assignment : assignments) {
        int column_index = findColumnIndex(assignment.column);

        if (column_index < 0) {
            throw DatabaseError(
                "Unknown column: " +
                assignment.column
            );
        }

        updated_values[column_index] = assignment.value;
    }

    validateColumnCount(updated_values);

    for (size_t i = 0; i < schema.size(); ++i) {
        validateColumnType(
            updated_values[i],
            schema[i]
        );

        validateNotNull(
            updated_values[i],
            schema[i]
        );
    }

    validateUniqueConstraints(
        updated_values,
        row.id
    );

    std::vector<Value> old_values = row.values;

    eraseFromIndexes(row);

    try {
        row.values = updated_values;
        insertIntoIndexes(row);
        appendUpdate(row);
        tryCompact();
    } catch (...) {
        row.values = old_values;
        insertIntoIndexes(row);
        throw;
    }
}

void Table::deleteRow(RowId row_id) {
    for (auto it = rows.begin(); it != rows.end(); ++it) {
        if ((*it)->id != row_id) {
            continue;
        }

        (*it)->deleted = true;
        eraseFromIndexes(*(*it));
        appendDelete(row_id);

        tryCompact();
        return;
    }
}

void Table::save() const {
    std::filesystem::path temp_path = storage_path;
    temp_path += ".tmp";

    std::ofstream file(temp_path, std::ios::trunc);

    if (!file.is_open()) {
        throw DatabaseError("Failed to open table file");
    }

    file << name << "\n";
    file << next_row_id << "\n";
    file << schema.size() << "\n";
    for (const auto& column : schema) {
        file
            << column.name << "|"
            << static_cast<int>(column.type) << "|"
            << column.not_null << "|"
            << column.indexed << "\n";
    }

    file << rows.size() << "\n";

    for (const auto& row_ptr : rows) {
        const Row& row = *row_ptr;

        file << row.id << "\n";
        file << row.values.size() << "\n";
        for (const auto& value : row.values) {
            file << Serializer::serializeValue(value) << "\n";
        }
    }

    file.flush();

    if (!file.good()) {
        throw DatabaseError("Failed to write table file");
    }

    file.close();

    if (std::filesystem::exists(storage_path)) {
        std::filesystem::remove(storage_path);
    }

    std::filesystem::rename(temp_path, storage_path);
}

void Table::load() {
    std::ifstream meta(metadata_path);

    if (!meta.is_open()) {
        throw DatabaseError("Failed to open metadata file");
    }

    schema.clear();
    rows.clear();
    row_lookup.clear();
    indexes.clear();

    size_t column_count = 0;
    meta >> column_count;

    std::string line;
    std::getline(meta, line);

    for (size_t i = 0; i < column_count; ++i) {
        std::getline(meta, line);

        std::stringstream ss(line);

        std::string name;
        std::string type;
        std::string not_null;
        std::string indexed;

        std::getline(ss, name, '|');
        std::getline(ss, type, '|');
        std::getline(ss, not_null, '|');
        std::getline(ss, indexed, '|');

        ColumnSchema column;

        column.name = name;
        column.type = static_cast<ColumnType>(std::stoi(type));
        column.not_null = static_cast<bool>(std::stoi(not_null));
        column.indexed = static_cast<bool>(std::stoi(indexed));

        schema.push_back(column);

        if (column.indexed) {
            indexes.emplace(column.name, BTree());
        }
    }

    std::ifstream data(data_path);

    if (!data.is_open()) {
        return;
    }

    RowId max_row_id = 0;

    while (std::getline(data, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);

        std::string op;
        std::getline(ss, op, '|');

        if (op == "D") {
            std::string row_id_token;

            std::getline(ss, row_id_token, '|');

            RowId row_id = std::stoull(row_id_token);
            Row* row = findRowById(row_id);

            if (row) {
                row->deleted = true;
                eraseFromIndexes(*row);
            }

            continue;
        }

        if (op != "I" && op != "U") {
            throw DatabaseError("Unknown operation type");
        }

        std::string row_id_token;
        std::getline(ss, row_id_token, '|');

        RowId row_id = std::stoull(row_id_token);

        max_row_id = std::max(max_row_id, row_id);

        std::vector<Value> values;
        for (size_t i = 0; i < schema.size(); ++i) {
            std::string token;
            std::getline(ss, token, '|');
            values.push_back(Serializer::deserializeValue(token));
        }

        if (op == "I") {
            auto row = std::make_unique<Row>();

            row->id = row_id;
            row->values = values;

            Row* row_ptr = row.get();

            rows.push_back(std::move(row));
            row_lookup[row_id] = row_ptr;
            insertIntoIndexes(*row_ptr);

            continue;
        }

        Row* row = findRowById(row_id);

        if (!row) {
            throw DatabaseError("Update for unknown row");
        }

        eraseFromIndexes(*row);
        row->values = values;
        insertIntoIndexes(*row);
    }

    next_row_id = max_row_id + 1;
}

void Table::compact() {
    std::filesystem::path temp_path = data_path;
    temp_path += ".tmp";

    std::ofstream file(temp_path);

    if (!file.is_open()) {
        throw DatabaseError("Failed to create compacted file");
    }

    for (const auto& row_ptr : rows) {
        const Row& row = *row_ptr;

        if (row.deleted) {
            continue;
        }

        file << "I|" << Serializer::serializeRow(row) << "\n";
    }

    file.close();

    std::filesystem::remove(data_path);
    std::filesystem::rename(temp_path, data_path);
}

void Table::tryCompact() {
    ++operation_count;

    constexpr size_t kCompactThreshold = 100;

    if (operation_count >= kCompactThreshold) {
        compact();
        operation_count = 0;
    }
}

void Table::validateRow(const std::vector<Value>& values) const {
    validateColumnCount(values);

    for (size_t i = 0; i < schema.size(); ++i) {
        validateColumnType(values[i], schema[i]);
        validateNotNull(values[i], schema[i]);
    }

    validateUniqueConstraints(values);
}

void Table::validateColumnCount(
    const std::vector<Value>& values
) const {
    if (values.size() != schema.size()) {
        throw DatabaseError("Column count does not match schema");
    }
}

void Table::validateColumnType(
    const Value& value,
    const ColumnSchema& column
) const {
    if (value.isNull()) {
        return;
    }

    if (
        column.type == ColumnType::kInt &&
        value.getType() != Value::Type::kInt
    ) {
        throw DatabaseError(
            "Expected INT value for column '" +
            column.name +
            "'"
        );
    }

    if (
        column.type == ColumnType::kString &&
        value.getType() != Value::Type::kString
    ) {
        throw DatabaseError(
            "Expected STRING value for column '" +
            column.name +
            "'"
        );
    }
}

void Table::validateNotNull(
    const Value& value,
    const ColumnSchema& column
) const {
    if (column.not_null && value.isNull()) {
        throw DatabaseError(
            "Column '" +
            column.name +
            "' cannot be NULL"
        );
    }
}

void Table::validateUniqueConstraints(
    const std::vector<Value>& values,
    RowId ignored_row_id
) const {
    for (size_t i = 0; i < schema.size(); ++i) {
        const auto& column = schema[i];

        if (!column.indexed) {
            continue;
        }

        auto index_it = indexes.find(column.name);
        if (index_it == indexes.end()) {
            continue;
        }

        RowId existing_row_id = index_it->second.find(
            IndexedValue(values[i])
        );

        if (existing_row_id == 0) {
            continue;
        }

        if (existing_row_id == ignored_row_id) {
            continue;
        }

        throw DatabaseError(
            "Duplicate value for indexed column '" +
            column.name +
            "'"
        );
    }
}

int Table::findColumnIndex(
    const std::string& column_name
) const {
    for (size_t i = 0; i < schema.size(); ++i) {
        if (schema[i].name == column_name) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

void Table::insertIntoIndexes(const Row& row) {
    for (size_t i = 0; i < schema.size(); ++i) {
        const auto& column = schema[i];

        if (!column.indexed) {
            continue;
        }

        indexes[column.name].insert(
            IndexedValue(row.values[i]),
            row.id
        );
    }
}

void Table::eraseFromIndexes(const Row& row) {
    for (size_t i = 0; i < schema.size(); ++i) {
        const auto& column = schema[i];

        if (!column.indexed) {
            continue;
        }

        indexes[column.name].erase(IndexedValue(row.values[i]));
    }
}


void Table::appendInsert(const Row& row) {
    std::ofstream file(data_path, std::ios::app);

    if (!file.is_open()) {
        throw DatabaseError("Failed to open table file");
    }

    file << "I|" << Serializer::serializeRow(row) << "\n";
}

void Table::appendDelete(RowId row_id) {
    std::ofstream file(data_path, std::ios::app);

    if (!file.is_open()) {
        throw DatabaseError("Failed to open table file");
    }

    file << "D|" << row_id << "\n";
}

void Table::appendUpdate(const Row& row) {
    std::ofstream file(data_path, std::ios::app);

    if (!file.is_open()) {
        throw DatabaseError("Failed to open table file");
    }

    file << "U|" << Serializer::serializeRow(row) << "\n";
}


void Table::saveSchema() const {
    std::ofstream file(metadata_path);

    if (!file.is_open()) {
        throw DatabaseError("Failed to open metadata file");
    }

    file << schema.size() << "\n";

    for (const auto& column : schema) {
        file
            << column.name << "|"
            << static_cast<int>(column.type) << "|"
            << column.not_null << "|"
            << column.indexed << "\n";
    }
}

} // namespace dbms

