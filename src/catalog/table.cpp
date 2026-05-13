#include "catalog/table.hpp"

#include "common/value_comparator.hpp"
#include "exceptions/database_error.hpp"

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

    Row row;

    row.id = next_row_id;
    ++next_row_id;
    row.values = values;

    rows.push_back(row);

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

void Table::validateRow(
    const std::vector<Value>& values
) const {
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
        throw DatabaseError(
            "Column count does not match schema");
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
            "'");
    }

    if (column.type == ColumnType::kString &&
        value.getType() != Value::Type::kString) {
        throw DatabaseError(
            "Expected STRING value for column '" +
            column.name +
            "'");
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
            "' cannot be NULL");
    }
}

void Table::validateUniqueConstraints(
    const std::vector<Value>& values
) const {
    for (size_t column_index = 0;
         column_index < schema.size();
         ++column_index) {

        const auto& column = schema[column_index];

        if (!column.indexed) {
            continue;
        }

        const Value& new_value =
            values[column_index];

        for (const auto& row : rows) {
            if (row.deleted) {
                continue;
            }

            const Value& existing_value =
                row.values[column_index];

            if (ValueComparator::compare(existing_value,
                                         new_value,
                                         "==")) {
                throw DatabaseError(
                    "Duplicate value for indexed column '" +
                    column.name +
                    "'");
            }
        }
    }
}

} // namespace dbms

