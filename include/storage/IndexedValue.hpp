#pragma once

#include "common/Value.hpp"

namespace dbms {

class IndexedValue {
public:
    IndexedValue() = default;

    explicit IndexedValue(const Value& value);

    const Value& GetValue() const;

    bool operator<(const IndexedValue& other) const;
    bool operator>(const IndexedValue& other) const;
    bool operator==(const IndexedValue& other) const;

private:
    Value value;
};

} // namespace dbms

