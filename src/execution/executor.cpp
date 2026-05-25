#include "execution/executor.hpp"

#include <iostream>
#include <regex>
#include <stdexcept>

#include "common/value_comparator.hpp"

namespace dbms {

Executor::Executor(System& system) :
    system(system)
{}

void Executor::execute(const Command& cmd) {
    switch (cmd.type) {

    case CommandType::kCreateDatabase:
        executeCreateDatabase(cmd);
        return;

    case CommandType::kDropDatabase:
        executeDropDatabase(cmd);
        return;

    case CommandType::kUseDatabase:
        executeUseDatabase(cmd);
        return;

    case CommandType::kCreateTable:
        executeCreateTable(cmd);
        return;

    case CommandType::kDropTable:
        executeDropTable(cmd);
        return;

    case CommandType::kInsert:
        executeInsert(cmd);
        return;

    case CommandType::kSelect:
        executeSelect(cmd);
        return;

    case CommandType::kUpdate:
        executeUpdate(cmd);
        return;

    case CommandType::kDelete:
        executeDelete(cmd);
        return;

    default:
        throw std::runtime_error("Unknown command");
    }
}

void Executor::executeCreateDatabase(const Command& cmd) {
    system.createDatabase(cmd.database_name);

    std::cout
        << "Database "
        << cmd.database_name
        << " created\n";
}

void Executor::executeDropDatabase(const Command& cmd) {
    Database* db = system.getDatabase(cmd.database_name);

    if (!db) {
        throw std::runtime_error("Database not found");
    }

    system.dropDatabase(cmd.database_name);

    std::cout
        << "Database dropped: "
        << cmd.database_name
        << "\n";
}

void Executor::executeUseDatabase(const Command& cmd) {
    system.useDatabase(cmd.database_name);

    std::cout
        << "Using database "
        << cmd.database_name
        << "\n";
}

void Executor::executeCreateTable(const Command& cmd) {
    Database* db = resolveDatabase(cmd);

    db->createTable(cmd.table_name, cmd.columns);

    std::cout
        << "Table "
        << cmd.table_name
        << " created\n";
}

void Executor::executeDropTable(const Command& cmd) {
    Database* db = resolveDatabase(cmd);
    Table* table = db->getTable(cmd.table_name);

    if (!table) {
        throw std::runtime_error("Table not found");
    }

    db->dropTable(cmd.table_name);

    std::cout
        << "Table dropped: "
        << cmd.table_name
        << "\n";
}

void Executor::executeInsert(const Command& cmd) {
    Database* db = resolveDatabase(cmd);
    
    Table* table = db->getTable(cmd.table_name);
    if (!table) {
        throw std::runtime_error("Table not found");
    }
    
    const auto& schema = table->getSchema();
    for (const auto& column_name : cmd.column_names) {
        validateColumnExists(
            schema,
            column_name
        );
    }


    for (const auto& values : cmd.values) {
        table->insertRow(buildInsertRow(*table, cmd, values));
    }
    
    std::cout << cmd.values.size() << " rows inserted\n";
}

void Executor::executeSelect(const Command& cmd) {
    Database* db = resolveDatabase(cmd);
    Table* table = db->getTable(cmd.table_name);

    if (!table) {
        throw std::runtime_error("Table not found");
    }

    const auto& schema = table->getSchema();
    
    validateSelectColumns(cmd, schema);
    validateConditions(cmd.conditions, schema);
    
    std::vector<const Row*> result_rows;
    if (canUseIndexLookup(*table, cmd.conditions)) {
        Row* row = tryFindIndexedRow(*table, cmd.conditions);

        if (row && matchConditions(
            cmd.conditions,
            *row,
            schema
        )) {
            result_rows.push_back(row);
        }

        printJsonRows(result_rows, cmd, schema);
        return;
    }

    const auto& rows = table->getRows();
    for (const auto& row_ptr : rows) {
        const Row& row = *row_ptr;

        if (row.deleted) {
            continue;
        }

        if (!matchConditions(cmd.conditions, row, schema)) {
            continue;
        }

        result_rows.push_back(&row);
    }

    printJsonRows(result_rows, cmd, schema);
}

void Executor::executeUpdate(const Command& cmd) {
    Database* db = resolveDatabase(cmd);

    Table* table = db->getTable(cmd.table_name);

    if (!table) {
        throw std::runtime_error("Table not found");
    }

    auto& rows = table->getRowsMutable();
    const auto& schema = table->getSchema();
    
    validateAssignments(cmd, schema);
    validateConditions(cmd.conditions, schema);
    
    size_t updated = 0;
    for (const auto& row_ptr : rows) {
        Row& row = *row_ptr;

        if (row.deleted) {
            continue;
        }

        if (!matchConditions(cmd.conditions, row, schema)) {
            continue;
        }

        table->updateRow(row, cmd.assignments);
        ++updated;
    }

    std::cout << updated << " rows updated\n";
}

void Executor::executeDelete(const Command& cmd) {
    Database* db = resolveDatabase(cmd);

    Table* table = db->getTable(cmd.table_name);

    if (!table) {
        throw std::runtime_error("Table not found");
    }

    auto& rows = table->getRowsMutable();
    const auto& schema = table->getSchema();

    validateConditions(cmd.conditions, schema);

    size_t deleted = 0;
    for (const auto& row_ptr : rows) {
        Row& row = *row_ptr;

        if (row.deleted) {
            continue;
        }

        if (!matchConditions(cmd.conditions, row, schema)) {
            continue;
        }

        table->deleteRow(row.id);
        ++deleted;
    }

    std::cout << deleted << " rows deleted\n";
}

Database* Executor::resolveDatabase(const Command& cmd) {
    Database* db = nullptr;

    if (!cmd.database_name.empty()) {
        db = system.getDatabase(cmd.database_name);
    } else {
        db = system.getCurrentDatabase();
    }

    if (!db) {
        throw std::runtime_error("No database selected");
    }

    return db;
}

void Executor::printJsonRows(
    const std::vector<const Row*>& rows,
    const Command& cmd,
    const std::vector<ColumnSchema>& schema
) {
    std::cout << "[\n";

    for (size_t i = 0; i < rows.size(); ++i) {
        printJsonRow(*rows[i], cmd, schema);

        if (i + 1 != rows.size()) {
            std::cout << ",";
        }

        std::cout << "\n";
    }

    std::cout << "]\n";
}

void Executor::printJsonRow(
    const Row& row,
    const Command& cmd,
    const std::vector<ColumnSchema>& schema
) {
    std::cout << "  {\n";

    bool select_all = cmd.select_columns.size() == 1 &&
        cmd.select_columns[0].name == "*";

    std::vector<SelectColumn> columns;
    if (select_all) {
        for (const auto& column : schema) {
            SelectColumn select_column;
            select_column.name = column.name;
            columns.push_back(select_column);
        }
    } else {
        columns = cmd.select_columns;
    }

    for (size_t i = 0; i < columns.size(); ++i) {
        const auto& column = columns[i];

        int column_index = findColumnIndex(
            schema,
            column.name
        );

        if (column_index < 0) {
            throw std::runtime_error(
                "Unknown column: " +
                column.name
            );
        }

        std::string field_name;
        if (column.alias.empty()) {
            field_name = column.name;
        } else {
            field_name = column.alias;
        }

        std::cout << "    \"" << field_name << "\": ";
        printJsonValue(row.values[column_index]);

        if (i + 1 != columns.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }

    std::cout << "  }";
}

void Executor::printJsonValue(const Value& value) {
    switch (value.getType()) {

    case Value::Type::kInt:
        std::cout << value.asInt();
        return;

    case Value::Type::kString:
        std::cout << "\"" << value.asString() << "\"";
        return;

    case Value::Type::kNull:
        std::cout << "NULL";
        return;
    }
}

bool Executor::matchConditions(
    const std::vector<Condition>& conditions,
    const Row& row,
    const std::vector<ColumnSchema>& schema
) {
    if (conditions.empty()) {
        return true;
    }

    for (const auto& condition : conditions) {
        Value left = resolveOperand(
            condition.lhs,
            row,
            schema
        );

        Value right = resolveOperand(
            condition.rhs,
            row,
            schema
        );

        if (condition.operator_type == "BETWEEN") {
            Value range_end = resolveOperand(
                condition.range_end,
                row,
                schema
            );

            if (!ValueComparator::between(
                left,
                right,
                range_end)
            ) {
                return false;
            }

            continue;
        }

        if (condition.operator_type == "LIKE") {
            if (!likeValues(left, right)) {
                return false;
            }

            continue;
        }

        if (!ValueComparator::compare(
            left,
            right,
            condition.operator_type)
        ) {
            return false;
        }
    }

    return true;
}

int Executor::findColumnIndex(
    const std::vector<ColumnSchema>& schema,
    const std::string& name
) {
    for (size_t i = 0; i < schema.size(); ++i) {
        if (schema[i].name == name) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

void Executor::validateColumnExists(
    const std::vector<ColumnSchema>& schema,
    const std::string& column_name
) {
    if (findColumnIndex(schema, column_name) < 0) {
        throw std::runtime_error(
            "Unknown column: " +
            column_name
        );
    }
}

void Executor::validateSelectColumns(
    const Command& cmd,
    const std::vector<ColumnSchema>& schema
) {
    bool select_all =
        cmd.select_columns.size() == 1 &&
        cmd.select_columns[0].name == "*";

    if (select_all) {
        return;
    }

    for (const auto& column : cmd.select_columns) {
        validateColumnExists(
            schema,
            column.name
        );
    }
}

void Executor::validateAssignments(
    const Command& cmd,
    const std::vector<ColumnSchema>& schema
) {
    for (const auto& assignment : cmd.assignments) {
        validateColumnExists(
            schema,
            assignment.column
        );
    }
}

void Executor::validateConditions(
    const std::vector<Condition>& conditions,
    const std::vector<ColumnSchema>& schema
) {
    for (const auto& condition : conditions) {
        if (condition.lhs.is_column) {
            validateColumnExists(
                schema,
                condition.lhs.column
            );
        }

        if (condition.rhs.is_column) {
            validateColumnExists(
                schema,
                condition.rhs.column
            );
        }

        if (condition.range_end.is_column) {
            validateColumnExists(
                schema,
                condition.range_end.column
            );
        }
    }
}

Value Executor::resolveOperand(
    const Operand& operand,
    const Row& row,
    const std::vector<ColumnSchema>& schema
) {
    if (!operand.is_column) {
        return operand.value;
    }

    int column_index = findColumnIndex(
        schema,
        operand.column
    );

    if (column_index < 0 || column_index >=
        static_cast<int>(row.values.size())
    ) {
        return Value();
    }

    return row.values[column_index];
}

bool Executor::likeValues(
    const Value& value,
    const Value& pattern
) {
    if (value.isNull() || pattern.isNull()) {
        return false;
    }

    if (value.getType() != Value::Type::kString ||
        pattern.getType() != Value::Type::kString
    ) {
        return false;
    }

    try {
        std::regex regex(pattern.asString());

        return std::regex_match(
            value.asString(),
            regex
        );
    } catch (...) {
        return false;
    }
}

bool Executor::canUseIndexLookup(
    const Table& table,
    const std::vector<Condition>& conditions
) {
    if (conditions.size() != 1) {
        return false;
    }

    const Condition& condition = conditions[0];

    if (condition.operator_type != "==") {
        return false;
    }

    if (!condition.lhs.is_column) {
        return false;
    }

    if (condition.rhs.is_column) {
        return false;
    }

    return table.hasIndexedColumn(
        condition.lhs.column
    );
}

Row* Executor::tryFindIndexedRow(
    Table& table,
    const std::vector<Condition>& conditions
) {
    const Condition& condition = conditions[0];

    return table.findByIndexedValue(
        condition.lhs.column,
        condition.rhs.value
    );
}

std::vector<Value> Executor::buildInsertRow(
    const Table& table,
    const Command& cmd,
    const std::vector<Value>& values
) {
    const auto& schema = table.getSchema();

    if (cmd.column_names.empty()) {
        return values;
    }

    if (cmd.column_names.size() != values.size()) {
        throw std::runtime_error(
            "Column count does not match value count"
        );
    }

    std::vector<Value> result(schema.size(), Value());

    for (size_t i = 0; i < cmd.column_names.size(); ++i) {
        int column_index = findColumnIndex(
            schema,
            cmd.column_names[i]
        );

        if (column_index < 0) {
            throw std::runtime_error(
                "Unknown column: " +
                cmd.column_names[i]
            );
        }

        result[column_index] = values[i];
    }

    return result;
}

} // namespace dbms

