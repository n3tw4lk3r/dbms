#include "common/value_comparator.hpp"

namespace dbms {

bool ValueComparator::Compare(
    const Value& lhs,
    const Value& rhs,
    std::string_view operator_str
) {
    if (lhs.IsNull() || rhs.IsNull()) {
        if (operator_str == "==") {
            return lhs.IsNull() && rhs.IsNull();
        }

        if (operator_str == "!=") {
            return lhs.IsNull() != rhs.IsNull();
        }

        return false;
    }

    if (lhs.GetType() != rhs.GetType()) {
        return false;
    }

    if (lhs.GetType() == Value::Type::kInt) {
        int x = lhs.AsInt();
        int y = rhs.AsInt();

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

    if (lhs.GetType() == Value::Type::kString) {
        const auto& x = lhs.AsString();
        const auto& y = rhs.AsString();

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

bool ValueComparator::Between(
    const Value& value,
    const Value& lhs,
    const Value& rhs
) {
    if (value.IsNull() || lhs.IsNull() || rhs.IsNull()) {
        return false;
    }

    if (
        value.GetType() != lhs.GetType() ||
            value.GetType() != rhs.GetType()
    ) {
        return false;
    }

    if (value.GetType() == Value::Type::kInt) {
        int x = value.AsInt();

        return lhs.AsInt() <= x && x < rhs.AsInt();
    }

    if (value.GetType() == Value::Type::kString) {
        const auto& x = value.AsString();
        return lhs.AsString() <= x && x < rhs.AsString();
    }

    return false;
}

} // namespace dbms

