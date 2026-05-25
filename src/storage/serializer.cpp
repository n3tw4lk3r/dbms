#include "storage/serializer.hpp"
#include "exceptions/database_error.hpp"

#include <sstream>

namespace dbms {

std::string Serializer::serializeValue(const Value& value) {
    switch (value.getType()) {

    case Value::Type::kNull:
        return "NULL";

    case Value::Type::kInt:
        return "INT:" + std::to_string(value.asInt());

    case Value::Type::kString: {
        const std::string& str = value.asString();
        return "STRING:" + std::to_string(str.size()) + ":" + str;
    }

    }

    throw DatabaseError("Unknown value type");
}

Value Serializer::deserializeValue(const std::string& token) {
    if (token == "NULL") {
        return Value();
    }

    if (token.rfind("INT:", 0) == 0) {
        return Value(std::stoi(token.substr(4)));
    }

    if (token.rfind("STRING:", 0) != 0) {
        throw DatabaseError("Corrupted value format");
    }

    size_t size_delimeter = token.find(':', 7);
    if (size_delimeter == std::string::npos) {
        throw DatabaseError("Corrupted string format");
    }

    size_t len = std::stoull(token.substr(7, size_delimeter - 7));
    std::string value = token.substr(size_delimeter + 1);

    if (value.size() != len) {
        throw DatabaseError("String size mismatch");
    }

    return Value(value);
}

std::string Serializer::serializeRowBody(const Row& row) {
    std::string out;

    for (size_t i = 0; i < row.values.size(); ++i) {
        if (i > 0) {
            out += "|";
        }
        out += serializeValue(row.values[i]);
    }

    return out;
}

std::string Serializer::serializeRow(const Row& row) {
    return std::to_string(row.id) + "|" + serializeRowBody(row);
}

Row Serializer::deserializeRow(const std::string& line) {
    std::stringstream ss(line);

    std::string id_token;
    std::getline(ss, id_token, '|');

    Row row;
    row.id = std::stoull(id_token);

    std::string token;
    while (std::getline(ss, token, '|')) {
        row.values.push_back(deserializeValue(token));
    }

    return row;
}

} // namespace dbms

