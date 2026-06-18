#include "catalog/table.hpp"

#include <fstream>

#include "exceptions/database_error.hpp"
#include "storage/indexed_value.hpp"
#include "storage/serializer.hpp"

namespace dbms {

Table::Table(
    std::string_view name,
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
    Load();
}

const std::string Table::GetName() const {
    return name;
}

RowId Table::InsertRow(const std::vector<Value>& values) {
    ValidateRow(values);

    auto row = std::make_unique<Row>();

    row->id = next_row_id;
    ++next_row_id;

    row->values = values;

    Row* row_ptr = row.get();

    rows.push_back(std::move(row));
    row_lookup[row_ptr->id] = row_ptr;

    InsertIntoIndexes(*row_ptr);
    AppendInsert(*row_ptr);

    TryCompact();
    return row_ptr->id;
}

const std::vector<std::unique_ptr<Row>>& Table::GetRows() const {
    return rows;
}

std::vector<std::unique_ptr<Row>>& Table::GetRowsMutable() {
    return rows;
}

const std::vector<ColumnSchema>& Table::GetSchema() const {
    return schema;
}

bool Table::HasIndexedColumn(
    const std::string& column_name
) const {
    return indexes.find(column_name) != indexes.end();
}

bool Table::ContainsIndexedValue(
    const std::string& column_name,
    const Value& value
) const {
    auto it = indexes.find(column_name);

    if (it == indexes.end()) {
        return false;
    }

    return it->second.Contains(
        IndexedValue(value)
    );
}

Row* Table::FindByIndexedValue(
    const std::string& column_name,
    const Value& value
) {
    auto it = indexes.find(column_name);

    if (it == indexes.end()) {
        return nullptr;
    }

    RowId row_id = it->second.Find(IndexedValue(value));

    if (row_id == 0) {
        return nullptr;
    }

    Row* row = FindRowById(row_id);

    if (!row || row->deleted) {
        return nullptr;
    }

    return row;
}

Row* Table::FindRowById(RowId row_id) {
    auto it = row_lookup.find(row_id);

    if (it == row_lookup.end()) {
        return nullptr;
    }

    return it->second;
}

void Table::UpdateRow(
    Row& row,
    const std::vector<Assignment>& assignments
) {
    std::vector<Value> updated_values = row.values;

    for (const auto& assignment : assignments) {
        int column_index = FindColumnIndex(assignment.column);

        if (column_index < 0) {
            throw SchemaError(
                "Unknown column: " +
                    assignment.column
            );
        }

        updated_values[column_index] = assignment.value;
    }

    ValidateColumnCount(updated_values);

    for (size_t i = 0; i < schema.size(); ++i) {
        ValidateColumnType(
            updated_values[i],
            schema[i]
        );

        ValidateNotNull(
            updated_values[i],
            schema[i]
        );
    }

    ValidateUniqueConstraints(
        updated_values,
        row.id
    );

    std::vector<Value> old_values = row.values;

    EraseFromIndexes(row);

    try {
        row.values = updated_values;
        InsertIntoIndexes(row);
        AppendUpdate(row);
        TryCompact();
    } catch (...) {
        row.values = old_values;
        InsertIntoIndexes(row);
        throw;
    }
}

void Table::DeleteRow(RowId row_id) {
    for (auto it = rows.begin(); it != rows.end(); ++it) {
        if ((*it)->id != row_id) {
            continue;
        }

        (*it)->deleted = true;
        EraseFromIndexes(*(*it));
        AppendDelete(row_id);

        TryCompact();
        return;
    }
}

void Table::Save() const {
    std::filesystem::path temp_path = storage_path;
    temp_path += ".tmp";

    std::ofstream file(temp_path, std::ios::trunc);

    if (!file.is_open()) {
        throw StorageError("Failed to open table file");
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
            file << Serializer::SerializeValue(value) << "\n";
        }
    }

    file.flush();

    if (!file.good()) {
        throw StorageError("Failed to write table file");
    }

    file.close();

    if (std::filesystem::exists(storage_path)) {
        std::filesystem::remove(storage_path);
    }

    std::filesystem::rename(temp_path, storage_path);
}

void Table::Load() {
    std::ifstream meta(metadata_path);

    if (!meta.is_open()) {
        throw StorageError("Failed to open metadata file");
    }

    schema.clear();
    rows.clear();
    row_lookup.clear();
    indexes.clear();

    size_t column_count = 0;

    if (!(meta >> column_count)) {
        throw DataCorruptionError("Corrupted metadata file");
    }

    std::string line;
    std::getline(meta, line);

    for (size_t i = 0; i < column_count; ++i) {
        if (!std::getline(meta, line)) {
            throw DataCorruptionError("Unexpected end of metadata file");
        }

        std::stringstream ss(line);

        std::string name;
        std::string type;
        std::string not_null;
        std::string indexed;

        if (
            !std::getline(ss, name, '|') ||
                !std::getline(ss, type, '|') ||
                !std::getline(ss, not_null, '|') ||
                !std::getline(ss, indexed, '|')
        ) {
            throw DataCorruptionError("Corrupted column metadata");
        }

        ColumnSchema column;

        try {
            column.name = name;
            column.type = static_cast<ColumnType>(std::stoi(type));
            column.not_null = static_cast<bool>(std::stoi(not_null));
            column.indexed = static_cast<bool>(std::stoi(indexed));
        } catch (...) {
            throw DataCorruptionError("Invalid column metadata");
        }

        schema.push_back(column);

        if (column.indexed) {
            indexes.emplace(column.name, BTree());
        }
    }

    std::ifstream data(data_path);

    if (!data.is_open()) {
        next_row_id = 1;
        return;
    }

    RowId max_row_id = 0;

    while (std::getline(data, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string op;

        if (!std::getline(ss, op, '|')) {
            throw DataCorruptionError("Corrupted operation record");
        }

        if (op == "D") {
            std::string row_id_token;

            if (!std::getline(ss, row_id_token, '|')) {
                throw DataCorruptionError("Corrupted delete record");
            }

            RowId row_id = 0;

            try {
                row_id = std::stoull(row_id_token);
            } catch (...) {
                throw DataCorruptionError("Invalid row id in delete record");
            }

            Row* row = FindRowById(row_id);

            if (row) {
                row->deleted = true;
                EraseFromIndexes(*row);
            }

            continue;
        }

        if (op != "I" && op != "U") {
            throw DataCorruptionError("Unknown operation type");
        }

        std::string serialized_row;

        if (!std::getline(ss, serialized_row)) {
            throw DataCorruptionError("Corrupted row record");
        }

        Row parsed;

        try {
            parsed = Serializer::DeserializeRow(serialized_row);
        } catch (const std::exception& error) {
            throw DataCorruptionError(
                std::string("Failed to deserialize row: ") +
                error.what()
            );
        }

        if (parsed.values.size() != schema.size()) {
            throw SchemaError("Row does not match schema");
        }

        max_row_id = std::max(max_row_id, parsed.id);

        if (op == "I") {
            auto row = std::make_unique<Row>();

            row->id = parsed.id;
            row->values = parsed.values;

            Row* row_ptr = row.get();

            rows.push_back(std::move(row));
            row_lookup[row_ptr->id] = row_ptr;

            InsertIntoIndexes(*row_ptr);

            continue;
        }

        Row* row = FindRowById(parsed.id);

        if (!row) {
            throw NotFoundError("Update for unknown row");
        }

        EraseFromIndexes(*row);

        row->values = parsed.values;

        InsertIntoIndexes(*row);
    }

    next_row_id = max_row_id + 1;
}

void Table::Compact() {
    std::filesystem::path temp_path = data_path;
    temp_path += ".tmp";

    std::ofstream file(temp_path);

    if (!file.is_open()) {
        throw StorageError("Failed to create compacted file");
    }

    for (const auto& row_ptr : rows) {
        const Row& row = *row_ptr;

        if (row.deleted) {
            continue;
        }

        file << "I|" << Serializer::SerializeRow(row) << "\n";
    }

    file.close();

    std::filesystem::remove(data_path);
    std::filesystem::rename(temp_path, data_path);
}

void Table::TryCompact() {
    ++operation_count;

    static constexpr size_t kCompactThreshold = 100;

    if (operation_count >= kCompactThreshold) {
        Compact();
        operation_count = 0;
    }
}

void Table::ValidateRow(const std::vector<Value>& values) const {
    ValidateColumnCount(values);

    for (size_t i = 0; i < schema.size(); ++i) {
        ValidateColumnType(values[i], schema[i]);
        ValidateNotNull(values[i], schema[i]);
    }

    ValidateUniqueConstraints(values);
}

void Table::ValidateColumnCount(const std::vector<Value>& values) const {
    if (values.size() != schema.size()) {
        throw SchemaError("Column count does not match schema");
    }
}

void Table::ValidateColumnType(
    const Value& value,
    const ColumnSchema& column
) const {
    if (value.IsNull()) {
        return;
    }

    if (
        column.type == ColumnType::kInt &&
            value.GetType() != Value::Type::kInt
    ) {
        throw TypeError(
            "Expected INT value for column '" +
                column.name +
                "'"
        );
    }

    if (
        column.type == ColumnType::kString &&
            value.GetType() != Value::Type::kString
    ) {
        throw TypeError(
            "Expected STRING value for column '" +
                column.name +
                "'"
        );
    }
}

void Table::ValidateNotNull(
    const Value& value,
    const ColumnSchema& column
) const {
    if ((column.not_null || column.indexed) && value.IsNull()) {
        throw ConstraintViolationError(
            "Column '" +
                column.name +
                "' cannot be NULL"
        );
    }
}

void Table::ValidateUniqueConstraints(
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

        RowId existing_row_id = index_it->second.Find(
            IndexedValue(values[i])
        );

        if (existing_row_id == 0) {
            continue;
        }

        if (existing_row_id == ignored_row_id) {
            continue;
        }

        throw ConstraintViolationError(
            "Duplicate value for indexed column '" +
                column.name +
                "'"
        );
    }
}

int Table::FindColumnIndex(std::string_view column_name) const {
    for (size_t i = 0; i < schema.size(); ++i) {
        if (schema[i].name == column_name) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

void Table::InsertIntoIndexes(const Row& row) {
    for (size_t i = 0; i < schema.size(); ++i) {
        const auto& column = schema[i];

        if (!column.indexed) {
            continue;
        }

        indexes[column.name].Insert(
            IndexedValue(row.values[i]),
            row.id
        );
    }
}

void Table::EraseFromIndexes(const Row& row) {
    for (size_t i = 0; i < schema.size(); ++i) {
        const auto& column = schema[i];

        if (!column.indexed) {
            continue;
        }

        indexes[column.name].Erase(IndexedValue(row.values[i]));
    }
}


void Table::AppendInsert(const Row& row) {
    std::ofstream file(data_path, std::ios::app);

    if (!file.is_open()) {
        throw StorageError("Failed to open table file");
    }

    file << "I|" << Serializer::SerializeRow(row) << "\n";
}

void Table::AppendDelete(RowId row_id) {
    std::ofstream file(data_path, std::ios::app);

    if (!file.is_open()) {
        throw StorageError("Failed to open table file");
    }

    file << "D|" << row_id << "\n";
}

void Table::AppendUpdate(const Row& row) {
    std::ofstream file(data_path, std::ios::app);

    if (!file.is_open()) {
        throw StorageError("Failed to open table file");
    }

    file << "U|" << Serializer::SerializeRow(row) << "\n";
}


void Table::SaveSchema() const {
    std::filesystem::path temp_path = metadata_path;
    temp_path += ".tmp";

    std::ofstream file(temp_path);

    if (!file.is_open()) {
        throw StorageError("Failed to open metadata file");
    }

    file << schema.size() << "\n";

    for (const auto& column : schema) {
        file
            << column.name << "|"
            << static_cast<int>(column.type) << "|"
            << column.not_null << "|"
            << column.indexed << "\n";
    }

    file.flush();

    if (!file.good()) {
        throw StorageError("Failed to write metadata");
    }

    file.close();

    if (std::filesystem::exists(metadata_path)) {
        std::filesystem::remove(metadata_path);
    }

    std::filesystem::rename(
        temp_path,
        metadata_path
    );
}

} // namespace dbms

