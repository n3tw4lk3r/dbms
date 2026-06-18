#pragma once

#include <vector>

#include "common/Value.hpp"
#include "storage/RowId.hpp"

namespace dbms {

struct Row {
    RowId id = 0;
    std::vector<Value> values;
    bool deleted = false;
};

} // namespace dbms

