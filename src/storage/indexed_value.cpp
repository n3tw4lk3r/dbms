#include "storage/indexed_value.hpp"

namespace dbms {

IndexedValue::IndexedValue(const Value& value) :
    value(value)
{}

const Value& IndexedValue::GetValue() const {
    return value;
}

bool IndexedValue::operator<(const IndexedValue& other) const {
    if (value.GetType() != other.value.GetType()) {
        return false;
    }

    if (value.GetType() == Value::Type::kInt) {
        return value.AsInt() < other.value.AsInt();
    }

    if (value.GetType() == Value::Type::kString) {
        return value.AsString() < other.value.AsString();
    }

    return false;
}

bool IndexedValue::operator>(const IndexedValue& other) const {
    return other < *this;
}

bool IndexedValue::operator==(const IndexedValue& other) const {
    if (value.GetType() != other.value.GetType()) {
        return false;
    }

    if (value.GetType() == Value::Type::kInt) {
        return value.AsInt() == other.value.AsInt();
    }

    if (value.GetType() == Value::Type::kString) {
        return value.AsString() == other.value.AsString();
    }

    return false;
}

} // namespace dbms

