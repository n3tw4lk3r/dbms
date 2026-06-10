#pragma once

#include <string>

#include "common/value.hpp"

namespace dbms {

class ValueComparator {
public:
    static bool Compare(
        const Value& lhs,
        const Value& rhs,
        const std::string& operator_str
    );

    static bool Between(
        const Value& value,
        const Value& lhs,
        const Value& rhs
    );
};

} // namespace dbms

