#pragma once

#include <string>

namespace dbms {

enum class ColumnType {
    kInt,
    kString
};

struct ColumnSchema {
    std::string name;
    ColumnType type;

    bool not_null = false;
    bool indexed = false;
};

} // namespace dbms

