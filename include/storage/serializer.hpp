#pragma once

#include <string>

#include "common/value.hpp"
#include "storage/row.hpp"

namespace dbms {

class Serializer {
public:
    static std::string serializeValue(const Value& value);
    static Value deserializeValue(const std::string& token);

    static std::string serializeRowBody(const Row& row);
    static std::string serializeRow(const Row& row);
    static Row deserializeRow(const std::string& line);
};

} // namespace dbms

