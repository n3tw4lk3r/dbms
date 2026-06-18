#pragma once

#include <string_view>

#include "common/value.hpp"

namespace dbms {

class ValueComparator {
public:
    static bool Compare(
        const Value& lhs,
        const Value& rhs,
        std::string_view operator_str
    );

    static bool Between(
        const Value& value,
        const Value& lhs,
        const Value& rhs
    );
};

} // namespace dbms

