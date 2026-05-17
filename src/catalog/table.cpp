#include "catalog/table.hpp"

#include "exceptions/database_error.hpp"
#include "storage/indexed_value.hpp"

namespace dbms {

Table::Table(
    const std::string& name,
    const std::vector<ColumnSchema>& schema
) :
    name(name),
    schema(schema)
{
    for (const auto& column : schema) {
        if (column.indexed) {
            indexes.emplace(column.name, BTree());
        }
    }
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

    if (column.type == ColumnType::kInt &&
        value.getType() != Value::Type::kInt) {
        throw DatabaseError(
            "Expected INT value for column '" +
            column.name +
            "'"
        );
    }

    if (column.type == ColumnType::kString &&
        value.getType() != Value::Type::kString) {
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
    if (column.not_null &&
        value.isNull()) {
        throw DatabaseError(
            "Column '" +
            column.name +
            "' cannot be NULL"
        );
    }
}

void Table::validateUniqueConstraints(
    const std::vector<Value>& values
) const {
    for (size_t i = 0; i < schema.size(); ++i) {
        const auto& column = schema[i];

        if (!column.indexed) {
            continue;
        }

        if (containsIndexedValue(
                column.name,
                values[i])
        ) {
            throw DatabaseError(
                "Duplicate value for indexed column '" +
                column.name +
                "'"
            );
        }
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

} // namespace dbms

