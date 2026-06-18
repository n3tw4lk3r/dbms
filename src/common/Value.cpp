#include "common/Value.hpp"

#include "exceptions/errors.hpp"

namespace dbms {

Value::Value() :
    type(Type::kNull),
    int_value(0)
{}

Value::Value(int value) :
    type(Type::kInt),
    int_value(value)
{}

Value::Value(std::string_view value) :
    type(Type::kString),
    int_value(0),
    string_value(value)
{}

Value::Type Value::GetType() const {
    return type;
}

int Value::AsInt() const {
    if (type != Type::kInt) {
        throw TypeError("Value is not INT");
    }

    return int_value;
}

const std::string Value::AsString() const {
    if (type != Type::kString) {
        throw TypeError("Value is not STRING");
    }

    return string_value;
}

bool Value::IsNull() const {
    return type == Type::kNull;
}

} // namespace dbms

