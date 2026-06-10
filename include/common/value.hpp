#pragma once

#include <string>

namespace dbms {

class Value {
public:
    enum class Type {
        kInt,
        kString,
        kNull
    };

    Value();
    explicit Value(int value);
    explicit Value(const std::string& value);

    Type GetType() const;

    int AsInt() const;
    const std::string AsString() const;

    bool IsNull() const;

private:
    Type type;
    int int_value;
    std::string string_value;
};

} // namespace dbms

