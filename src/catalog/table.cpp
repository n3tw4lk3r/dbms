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
            indexes.emplace(
                column.name,
                BTree()
            );
        }
    }
}

const std::string Table::getName() const {
    return name;
}

RowId Table::insertRow(const std::vector<Value>& values) {
    validateRow(values);

    Row row;
    row.id = next_row_id;
    ++next_row_id;
    row.values = values;

    rows.push_back(row);
    insertIntoIndexes(rows.back());
    return row.id;
}

const std::vector<Row>& Table::getRows() const {
    return rows;
}

std::vector<Row>& Table::getRowsMutable() {
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

    RowId row_id = it->second.find(
            IndexedValue(value)
        );

    if (row_id == 0) {
        return nullptr;
    }

    for (auto& row : rows) {
        if (row.deleted) {
            continue;
        }

        if (row.id == row_id) {
            return &row;
        }
    }

    return nullptr;
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

void Table::insertIntoIndexes(
    const Row& row
) {
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

