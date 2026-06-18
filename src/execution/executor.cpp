#include "execution/executor.hpp"

#include <iostream>
#include <regex>

#include "common/value_comparator.hpp"
#include "exceptions/database_error.hpp"

namespace dbms {

Executor::Executor(System& system) :
    system(system)
{}

void Executor::Execute(const Command& cmd) {
    switch (cmd.type) {

    case CommandType::kCreateDatabase:
        ExecuteCreateDatabase(cmd);
        return;

    case CommandType::kDropDatabase:
        ExecuteDropDatabase(cmd);
        return;

    case CommandType::kUseDatabase:
        ExecuteUseDatabase(cmd);
        return;

    case CommandType::kCreateTable:
        ExecuteCreateTable(cmd);
        return;

    case CommandType::kDropTable:
        ExecuteDropTable(cmd);
        return;

    case CommandType::kInsert:
        ExecuteInsert(cmd);
        return;

    case CommandType::kSelect:
        ExecuteSelect(cmd);
        return;

    case CommandType::kUpdate:
        ExecuteUpdate(cmd);
        return;

    case CommandType::kDelete:
        ExecuteDelete(cmd);
        return;

    default:
        throw ExecutionError("Unknown command");
    }
}

void Executor::ExecuteCreateDatabase(const Command& cmd) {
    system.CreateDatabase(cmd.database_name);

    std::cout << "Database " << cmd.database_name << " created\n";
}

void Executor::ExecuteDropDatabase(const Command& cmd) {
    Database* db = system.GetDatabase(cmd.database_name);

    if (!db) {
        throw NotFoundError("Database not found");
    }

    system.DropDatabase(cmd.database_name);

    std::cout << "Database dropped: " << cmd.database_name << "\n";
}

void Executor::ExecuteUseDatabase(const Command& cmd) {
    system.UseDatabase(cmd.database_name);
    std::cout << "Using database " << cmd.database_name << "\n";
}

void Executor::ExecuteCreateTable(const Command& cmd) {
    Database* db = ResolveDatabase(cmd);
    db->CreateTable(cmd.table_name, cmd.columns);

    std::cout << "Table " << cmd.table_name << " created\n";
}

void Executor::ExecuteDropTable(const Command& cmd) {
    Database* db = ResolveDatabase(cmd);
    Table* table = db->GetTable(cmd.table_name);

    if (!table) {
        throw NotFoundError("Table not found");
    }

    db->DropTable(cmd.table_name);
    std::cout << "Table dropped: " << cmd.table_name << "\n";
}

void Executor::ExecuteInsert(const Command& cmd) {
    Database* db = ResolveDatabase(cmd);
    Table* table = db->GetTable(cmd.table_name);

    if (!table) {
        throw NotFoundError("Table not found");
    }
    
    const auto& schema = table->GetSchema();
    for (const auto& column_name : cmd.column_names) {
        ValidateColumnExists(schema, column_name);
    }


    for (const auto& values : cmd.values) {
        table->InsertRow(BuildInsertRow(*table, cmd, values));
    }
    
    std::cout << cmd.values.size() << " rows inserted\n";
}

void Executor::ExecuteSelect(const Command& cmd) {
    Database* db = ResolveDatabase(cmd);
    Table* table = db->GetTable(cmd.table_name);

    if (!table) {
        throw NotFoundError("Table not found");
    }

    const auto& schema = table->GetSchema();
    
    ValidateSelectColumns(cmd, schema);
    ValidateConditions(cmd.conditions, schema);
    
    std::vector<const Row*> result_rows;
    if (CanUseIndexLookup(*table, cmd.conditions)) {
        Row* row = TryFindIndexedRow(*table, cmd.conditions);

        if (
            row && MatchConditions(
                cmd.conditions,
                *row,
                schema
            )
        ) {
            result_rows.push_back(row);
        }

        PrintJsonRows(result_rows, cmd, schema);
        return;
    }

    const auto& rows = table->GetRows();
    for (const auto& row_ptr : rows) {
        const Row& row = *row_ptr;

        if (row.deleted) {
            continue;
        }

        if (!MatchConditions(cmd.conditions, row, schema)) {
            continue;
        }

        result_rows.push_back(&row);
    }

    PrintJsonRows(result_rows, cmd, schema);
}

void Executor::ExecuteUpdate(const Command& cmd) {
    Database* db = ResolveDatabase(cmd);
    Table* table = db->GetTable(cmd.table_name);

    if (!table) {
        throw NotFoundError("Table not found");
    }

    auto& rows = table->GetRowsMutable();
    const auto& schema = table->GetSchema();
    
    ValidateAssignments(cmd, schema);
    ValidateConditions(cmd.conditions, schema);
    
    size_t updated = 0;
    for (const auto& row_ptr : rows) {
        Row& row = *row_ptr;

        if (row.deleted) {
            continue;
        }

        if (!MatchConditions(cmd.conditions, row, schema)) {
            continue;
        }

        table->UpdateRow(row, cmd.assignments);
        ++updated;
    }

    std::cout << updated << " rows updated\n";
}

void Executor::ExecuteDelete(const Command& cmd) {
    Database* db = ResolveDatabase(cmd);
    Table* table = db->GetTable(cmd.table_name);

    if (!table) {
        throw NotFoundError("Table not found");
    }

    auto& rows = table->GetRowsMutable();
    const auto& schema = table->GetSchema();

    ValidateConditions(cmd.conditions, schema);

    size_t deleted = 0;
    for (const auto& row_ptr : rows) {
        Row& row = *row_ptr;

        if (row.deleted) {
            continue;
        }

        if (!MatchConditions(cmd.conditions, row, schema)) {
            continue;
        }

        table->DeleteRow(row.id);
        ++deleted;
    }

    std::cout << deleted << " rows deleted\n";
}

Database* Executor::ResolveDatabase(const Command& cmd) {
    Database* db = nullptr;

    if (!cmd.database_name.empty()) {
        db = system.GetDatabase(cmd.database_name);
    } else {
        db = system.GetCurrentDatabase();
    }

    if (!db) {
        throw ExecutionError("No database selected");
    }

    return db;
}

void Executor::PrintJsonRows(
    const std::vector<const Row*>& rows,
    const Command& cmd,
    const std::vector<ColumnSchema>& schema
) {
    std::cout << "[\n";

    for (size_t i = 0; i < rows.size(); ++i) {
        PrintJsonRow(*rows[i], cmd, schema);

        if (i + 1 != rows.size()) {
            std::cout << ",";
        }

        std::cout << "\n";
    }

    std::cout << "]\n";
}

void Executor::PrintJsonRow(
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

        int column_index = FindColumnIndex(
            schema,
            column.name
        );

        if (column_index < 0) {
            throw SchemaError(
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
        PrintJsonValue(row.values[column_index]);

        if (i + 1 != columns.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }

    std::cout << "  }";
}

void Executor::PrintJsonValue(const Value& value) {
    switch (value.GetType()) {

    case Value::Type::kInt:
        std::cout << value.AsInt();
        return;

    case Value::Type::kNull:
        std::cout << "null";
        return;

    case Value::Type::kString: {
        std::cout << "\"";

        const std::string str = value.AsString();

        for (char ch : str) {
            switch (ch) {

            case '\\':
                std::cout << "\\\\";
                break;

            case '"':
                std::cout << "\\\"";
                break;

            case '\n':
                std::cout << "\\n";
                break;

            case '\r':
                std::cout << "\\r";
                break;

            case '\t':
                std::cout << "\\t";
                break;

            default:
                std::cout << ch;
                break;
            }
        }

        std::cout << "\"";
        return;
    }

    }
}

bool Executor::MatchConditions(
    const std::vector<Condition>& conditions,
    const Row& row,
    const std::vector<ColumnSchema>& schema
) {
    if (conditions.empty()) {
        return true;
    }

    for (const auto& condition : conditions) {
        Value left = ResolveOperand(
            condition.lhs,
            row,
            schema
        );

        Value right = ResolveOperand(
            condition.rhs,
            row,
            schema
        );

        if (condition.operator_type == "BETWEEN") {
            Value range_end = ResolveOperand(
                condition.range_end,
                row,
                schema
            );

            if (
                !ValueComparator::Between(
                    left,
                    right,
                    range_end
                )
            ) {
                return false;
            }

            continue;
        }

        if (condition.operator_type == "LIKE") {
            if (!LikeValues(left, right)) {
                return false;
            }

            continue;
        }

        if (
            !ValueComparator::Compare(
                left,
                right,
                condition.operator_type
            )
        ) {
            return false;
        }
    }

    return true;
}

int Executor::FindColumnIndex(
    const std::vector<ColumnSchema>& schema,
    std::string_view name
) {
    for (size_t i = 0; i < schema.size(); ++i) {
        if (schema[i].name == name) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

void Executor::ValidateColumnExists(
    const std::vector<ColumnSchema>& schema,
    const std::string& column_name
) {
    if (FindColumnIndex(schema, column_name) < 0) {
        throw SchemaError(
            "Unknown column: " +
            column_name
        );
    }
}

void Executor::ValidateSelectColumns(
    const Command& cmd,
    const std::vector<ColumnSchema>& schema
) {
    bool select_all = cmd.select_columns.size() == 1 &&
        cmd.select_columns[0].name == "*";

    if (select_all) {
        return;
    }

    for (const auto& column : cmd.select_columns) {
        ValidateColumnExists(
            schema,
            column.name
        );
    }
}

void Executor::ValidateAssignments(
    const Command& cmd,
    const std::vector<ColumnSchema>& schema
) {
    for (const auto& assignment : cmd.assignments) {
        ValidateColumnExists(
            schema,
            assignment.column
        );
    }
}

void Executor::ValidateConditions(
    const std::vector<Condition>& conditions,
    const std::vector<ColumnSchema>& schema
) {
    for (const auto& condition : conditions) {
        if (condition.lhs.is_column) {
            ValidateColumnExists(
                schema,
                condition.lhs.column
            );
        }

        if (condition.rhs.is_column) {
            ValidateColumnExists(
                schema,
                condition.rhs.column
            );
        }

        if (condition.range_end.is_column) {
            ValidateColumnExists(
                schema,
                condition.range_end.column
            );
        }
    }
}

Value Executor::ResolveOperand(
    const Operand& operand,
    const Row& row,
    const std::vector<ColumnSchema>& schema
) {
    if (!operand.is_column) {
        return operand.value;
    }

    int column_index = FindColumnIndex(
        schema,
        operand.column
    );

    if (
        column_index < 0 || column_index >=
            static_cast<int>(row.values.size())
    ) {
        return Value();
    }

    return row.values[column_index];
}

bool Executor::LikeValues(
    const Value& value,
    const Value& pattern
) {
    if (value.IsNull() || pattern.IsNull()) {
        return false;
    }

    if (
        value.GetType() != Value::Type::kString ||
            pattern.GetType() != Value::Type::kString
    ) {
        return false;
    }

    try {
        std::regex regex(pattern.AsString());

        return std::regex_match(
            value.AsString(),
            regex
        );

    } catch (const std::regex_error& error) {
        throw ExecutionError(
            "Invalid regex pattern: " +
            std::string(error.what())
        );
    }
}

bool Executor::CanUseIndexLookup(
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

    return table.HasIndexedColumn(
        condition.lhs.column
    );
}

Row* Executor::TryFindIndexedRow(
    Table& table,
    const std::vector<Condition>& conditions
) {
    const Condition& condition = conditions[0];

    return table.FindByIndexedValue(
        condition.lhs.column,
        condition.rhs.value
    );
}

std::vector<Value> Executor::BuildInsertRow(
    const Table& table,
    const Command& cmd,
    const std::vector<Value>& values
) {
    const auto& schema = table.GetSchema();

    if (cmd.column_names.empty()) {
        return values;
    }

    if (cmd.column_names.size() != values.size()) {
        throw ExecutionError(
            "Column count does not match value count"
        );
    }

    std::vector<Value> result(schema.size(), Value());

    for (size_t i = 0; i < cmd.column_names.size(); ++i) {
        int column_index = FindColumnIndex(
            schema,
            cmd.column_names[i]
        );

        if (column_index < 0) {
            throw SchemaError(
                "Unknown column: " +
                cmd.column_names[i]
            );
        }

        result[column_index] = values[i];
    }

    return result;
}

} // namespace dbms

