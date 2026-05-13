#include "common/value_comparator.hpp"

namespace dbms {

bool ValueComparator::compare(
    const Value& lhs,
    const Value& rhs,
    const std::string& operator_str
) {
    if (lhs.getType() != rhs.getType()) {
        return false;
    }

    if (lhs.getType() == Value::Type::kInt) {
        int x = lhs.asInt();
        int y = rhs.asInt();

        if (operator_str == "==") {
            return x == y;
        }

        if (operator_str == "!=") {
            return x != y;
        }

        if (operator_str == "<") {
            return x < y;
        }

        if (operator_str == ">") {
            return x > y;
        }

        if (operator_str == "<=") {
            return x <= y;
        }

        if (operator_str == ">=") {
            return x >= y;
        }
    }

    if (lhs.getType() == Value::Type::kString) {
        const auto& x = lhs.asString();
        const auto& y = rhs.asString();

        if (operator_str == "==") {
            return x == y;
        }

        if (operator_str == "!=") {
            return x != y;
        }

        if (operator_str == "<") {
            return x < y;
        }

        if (operator_str == ">") {
            return x > y;
        }

        if (operator_str == "<=") {
            return x <= y;
        }

        if (operator_str == ">=") {
            return x >= y;
        }
    }

    return false;
}

bool ValueComparator::between(
    const Value& value,
    const Value& lhs,
    const Value& rhs
) {
    if (value.getType() != lhs.getType() ||
        value.getType() != rhs.getType()) {
        return false;
    }

    if (value.getType() == Value::Type::kInt) {
        int x = value.asInt();

        return lhs.asInt() <= x &&
               x < rhs.asInt();
    }

    if (value.getType() == Value::Type::kString) {
        const auto& x = value.asString();

        return lhs.asString() <= x &&
               x < rhs.asString();
    }

    return false;
}

} // namespace dbms

