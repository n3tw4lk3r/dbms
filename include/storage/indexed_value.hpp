#pragma once

#include "common/value.hpp"

namespace dbms {

class IndexedValue {
public:
    IndexedValue() = default;

    explicit IndexedValue(const Value& value);

    const Value& getValue() const;

    bool operator<(const IndexedValue& other) const;
    bool operator>(const IndexedValue& other) const;
    bool operator==(const IndexedValue& other) const;

private:
    Value value;
};

} // namespace dbms

