#pragma once

#include <string>
#include <vector>

namespace dbms {

class QueryBuffer {
public:
    std::vector<std::string> append(const std::string& line);
    bool empty() const;

private:
    std::string buffer;
    bool in_string = false;
    bool escaped = false;
};

} // namespace dbms

