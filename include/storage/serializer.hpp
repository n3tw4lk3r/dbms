#pragma once

#include <string>

#include "common/value.hpp"
#include "storage/row.hpp"

namespace dbms {

class Serializer {
public:
    static std::string SerializeValue(const Value& value);
    static Value DeserializeValue(const std::string& token);

    static std::string SerializeRowBody(const Row& row);
    static std::string SerializeRow(const Row& row);
    static Row DeserializeRow(const std::string& line);
};

} // namespace dbms

