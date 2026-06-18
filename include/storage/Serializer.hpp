#pragma once

#include <string_view>

#include "common/Value.hpp"
#include "storage/Row.hpp"

namespace dbms {

class Serializer {
public:
    static std::string SerializeValue(const Value& value);
    static Value DeserializeValue(std::string_view token);

    static std::string SerializeRowBody(const Row& row);
    static std::string SerializeRow(const Row& row);
    static Row DeserializeRow(std::string_view line);
};

} // namespace dbms

