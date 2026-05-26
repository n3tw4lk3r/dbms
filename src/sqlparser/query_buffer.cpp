#include "sqlparser/query_buffer.hpp"

namespace dbms {

std::vector<std::string> QueryBuffer::append(
    const std::string& line
) {
    buffer += line;
    buffer += '\n';

    std::vector<std::string> queries;

    size_t start = 0;

    bool in_string = false;
    bool escaped = false;

    for (size_t i = 0; i < buffer.size(); ++i) {
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
            queries.push_back(
                buffer.substr(start, i - start)
            );

            start = i + 1;
        }
    }

    buffer = buffer.substr(start);

    return queries;
}

} // namespace dbms
