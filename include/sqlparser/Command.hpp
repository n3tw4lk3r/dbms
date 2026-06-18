#pragma once

#include <string>
#include <vector>

#include "common/Value.hpp"
#include "common/types.hpp"

namespace dbms {

enum class CommandType {
    kCreateDatabase,
    kDropDatabase,
    kUseDatabase,
    kCreateTable,
    kDropTable,
    kInsert,
    kSelect,
    kUpdate,
    kDelete,
    kUnknown
};

struct Assignment {
    std::string column;
    Value value;
};

struct Operand {
    bool is_column = false;
    std::string column;
    Value value;
};

struct Condition {
    std::string operator_type;
    Operand lhs;
    Operand rhs;
    Operand range_end;
};

struct SelectColumn {
    std::string name;
    std::string alias;
};

struct Command {
    CommandType type = CommandType::kUnknown;

    std::string database_name;
    std::string table_name;

    std::vector<ColumnSchema> columns;
    std::vector<std::string> column_names;

    std::vector<std::vector<Value>> values;

    std::vector<Assignment> assignments;
    std::vector<SelectColumn> select_columns;
    std::vector<Condition> conditions;
};

} // namespace dbms

