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
    resolveDatabase(cmd);

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

    for (const auto& values : cmd.values) {
        table->insertRow(values);
    }

    std::cout
        << cmd.values.size()
        << " rows inserted\n";
}

void Executor::executeSelect(const Command& cmd) {
    Database* db = resolveDatabase(cmd);

    Table* table = db->getTable(cmd.table_name);

    if (!table) {
        throw std::runtime_error("Table not found");
    }

    const auto& schema = table->getSchema();

    if (canUseIndexLookup(*table, cmd.conditions)) {
        Row* row = tryFindIndexedRow(
            *table,
            cmd.conditions
        );

        if (!row) {
            std::cout << "(empty)\n";
            return;
        }

        if (!matchConditions(cmd.conditions, *row, schema)) {
            std::cout << "(empty)\n";
            return;
        }

        printSelectedRow(*row, cmd, schema);

        std::cout << '\n';
        return;
    }

    const auto& rows = table->getRows();

    bool has_rows = false;

    for (const auto& row : rows) {
        if (row.deleted) {
            continue;
        }

        if (!matchConditions(cmd.conditions, row, schema)) {
            continue;
        }

        has_rows = true;

        printSelectedRow(row, cmd, schema);

        std::cout << '\n';
    }

    if (!has_rows) {
        std::cout << "(empty)\n";
    }
}

void Executor::executeUpdate(const Command& cmd) {
    Database* db = resolveDatabase(cmd);

    Table* table = db->getTable(cmd.table_name);

    if (!table) {
        throw std::runtime_error("Table not found");
    }

    auto& rows = table->getRowsMutable();
    const auto& schema = table->getSchema();

    size_t updated = 0;

    for (auto& row : rows) {
        if (row.deleted) {
            continue;
        }

        if (!matchConditions(cmd.conditions, row, schema)) {
            continue;
        }

        for (const auto& assignment : cmd.assignments) {
            int column_index = findColumnIndex(
                schema,
                assignment.column
            );

            if (column_index < 0) {
                throw std::runtime_error(
                    "Unknown column: " +
                    assignment.column
                );
            }

            row.values[column_index] = assignment.value;
        }

        ++updated;
    }

    std::cout
        << updated
        << " rows updated\n";
}

void Executor::executeDelete(const Command& cmd) {
    Database* db = resolveDatabase(cmd);

    Table* table = db->getTable(cmd.table_name);

    if (!table) {
        throw std::runtime_error("Table not found");
    }

    auto& rows = table->getRowsMutable();
    const auto& schema = table->getSchema();

    size_t deleted = 0;

    for (auto& row : rows) {
        if (row.deleted) {
            continue;
        }

        if (!matchConditions(cmd.conditions, row, schema)) {
            continue;
        }

        row.deleted = true;

        ++deleted;
    }

    std::cout
        << deleted
        << " rows deleted\n";
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

void Executor::printFullRow(const Row& row) {
    for (size_t i = 0; i < row.values.size(); ++i) {
        printValue(row.values[i]);

        if (i + 1 != row.values.size()) {
            std::cout << ", ";
        }
    }
}

void Executor::printSelectedColumns(
    const Row& row,
    const std::vector<ColumnSchema>& schema,
    const std::vector<SelectColumn>& columns
) {
    for (size_t i = 0; i < columns.size(); ++i) {
        int column_index = findColumnIndex(
            schema,
            columns[i].name
        );

        if (column_index < 0) {
            throw std::runtime_error(
                "Unknown column: " +
                columns[i].name
            );
        }

        printValue(row.values[column_index]);

        if (i + 1 != columns.size()) {
            std::cout << ", ";
        }
    }
}

void Executor::printSelectedRow(
    const Row& row,
    const Command& cmd,
    const std::vector<ColumnSchema>& schema
) {
    bool select_all =
        cmd.select_columns.size() == 1 &&
        cmd.select_columns[0].name == "*";

    if (select_all) {
        printFullRow(row);
        return;
    }

    printSelectedColumns(
        row,
        schema,
        cmd.select_columns
    );
}

void Executor::printValue(const Value& value) {
    switch (value.getType()) {

    case Value::Type::kInt:
        std::cout << value.asInt();
        return;

    case Value::Type::kString:
        std::cout << value.asString();
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

} // namespace dbms

