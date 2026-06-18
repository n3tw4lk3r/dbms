#include "sqlparser/QueryBuffer.hpp"

namespace dbms {

std::vector<std::string> QueryBuffer::Append(std::string_view line) {
    buffer += line;
    buffer += '\n';

    std::vector<std::string> queries;
    std::size_t query_start = 0;

    for (std::size_t i = 0; i < buffer.size(); ++i) {
        char ch = buffer[i];

        if (escaped) {
            escaped = false;
            continue;
        }

        if (ch == '\\') {
            escaped = true;
            continue;
        }

        if (ch == '"') {
            in_string = !in_string;
            continue;
        }

        if (ch == ';' && !in_string) {
            queries.emplace_back(
                buffer.substr(query_start, i - query_start)
            );

            query_start = i + 1;
        }
    }

    buffer.erase(0, query_start);
    if (buffer.find_first_not_of(" \t\r\n") == std::string::npos) {
        buffer.clear();
    }
    return queries;
}

bool QueryBuffer::Empty() const {
    return buffer.empty();
}

} // namespace dbms

