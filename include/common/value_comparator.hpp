#pragma once

#include <string>

#include "common/value.hpp"

namespace dbms {

class ValueComparator {
public:
    static bool compare(
        const Value& lhs,
        const Value& rhs,
        const std::string& operator_str
    );

    static bool between(
        const Value& value,
        const Value& lhs,
        const Value& rhs
    );
};

} // namespace dbms

