#include "storage/indexed_value.hpp"

namespace dbms {

IndexedValue::IndexedValue(const Value& value) :
    value(value)
{}

const Value& IndexedValue::getValue() const {
    return value;
}

bool IndexedValue::operator<(
    const IndexedValue& other
) const {
    if (value.getType() != other.value.getType()) {
        return false;
    }

    if (value.getType() == Value::Type::kInt) {
        return value.asInt() < other.value.asInt();
    }

    if (value.getType() == Value::Type::kString) {
        return value.asString() < other.value.asString();
    }

    return false;
}

bool IndexedValue::operator>(
    const IndexedValue& other
) const {
    return other < *this;
}

bool IndexedValue::operator==(
    const IndexedValue& other
) const {
    if (value.getType() != other.value.getType()) {
        return false;
    }

    if (value.getType() == Value::Type::kInt) {
        return value.asInt() == other.value.asInt();
    }

    if (value.getType() == Value::Type::kString) {
        return value.asString() == other.value.asString();
    }

    return false;
}

} // namespace dbms

