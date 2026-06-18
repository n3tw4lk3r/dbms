#include "storage/serializer.hpp"

#include "exceptions/database_error.hpp"

namespace dbms {

std::string Serializer::SerializeValue(const Value& value) {
    switch (value.GetType()) {

    case Value::Type::kNull:
        return "NULL";

    case Value::Type::kInt:
        return "INT:" + std::to_string(value.AsInt());

    case Value::Type::kString: {
        const std::string& str = value.AsString();
        return "STRING:" + std::to_string(str.size()) + ":" + str;
    }

    }

    throw TypeError("Unknown value type");
}

Value Serializer::DeserializeValue(std::string_view token) {
    if (token == "NULL") {
        return Value();
    }

    if (token.rfind("INT:", 0) == 0) {
        return Value(std::stoi(std::string(token.substr(4))));
    }

    if (token.rfind("STRING:", 0) != 0) {
        throw DataCorruptionError("Corrupted value format");
    }

    size_t size_delimeter = token.find(':', 7);
    if (size_delimeter == std::string::npos) {
        throw DataCorruptionError("Corrupted string format");
    }

    size_t len = std::stoull(std::string(token.substr(7, size_delimeter - 7)));
    std::string value(token.substr(size_delimeter + 1));

    if (value.size() != len) {
        throw DataCorruptionError("String size mismatch");
    }

    return Value(value);
}

std::string Serializer::SerializeRowBody(const Row& row) {
    std::string out;

    for (size_t i = 0; i < row.values.size(); ++i) {
        if (i > 0) {
            out += "|";
        }
        out += SerializeValue(row.values[i]);
    }

    return out;
}

std::string Serializer::SerializeRow(const Row& row) {
    return std::to_string(row.id) + "|" + SerializeRowBody(row);
}

Row Serializer::DeserializeRow(std::string_view line) {
    Row row;

    size_t pos = line.find('|');

    if (pos == std::string::npos) {
        throw DataCorruptionError("Corrupted row");
    }

    row.id = std::stoull(std::string(line.substr(0, pos)));

    size_t current = pos + 1;
    while (current < line.size()) {
        if (line.compare(current, 4, "INT:") == 0) {
            size_t next = line.find('|', current);

            std::string token;

            if (next == std::string::npos) {
                token = line.substr(current);
                current = line.size();
            } else {
                token = line.substr(current, next - current);
                current = next + 1;
            }

            row.values.push_back(DeserializeValue(token));
            continue;
        }

        if (line.compare(current, 4, "NULL") == 0) {
            row.values.push_back(Value());

            size_t next = line.find('|', current);
            if (next == std::string::npos) {
                current = line.size();
            } else {
                current = next + 1;
            }

            continue;
        }

        if (line.compare(current, 7, "STRING:") == 0) {
            size_t len_begin = current + 7;
            size_t len_end = line.find(':', len_begin);

            if (len_end == std::string::npos) {
                throw DataCorruptionError("Corrupted string token");
            }

            size_t len = std::stoull(
                std::string(line.substr(len_begin, len_end - len_begin))
            );

            size_t data_begin = len_end + 1;

            if (data_begin + len > line.size()) {
                throw DataCorruptionError("Corrupted string length");
            }

            std::string value(line.substr(
                data_begin,
                len
            ));

            row.values.push_back(Value(value));

            current = data_begin + len;

            if (current < line.size() && line[current] == '|') {
                ++current;
            }

            continue;
        }

        throw DataCorruptionError("Unknown serialized token");
    }

    return row;
}

} // namespace dbms

