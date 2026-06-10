#pragma once

#include <string>
#include <vector>

namespace dbms {

class QueryBuffer {
public:
    std::vector<std::string> Append(const std::string& line);
    bool Empty() const;

private:
    std::string buffer;
    bool in_string = false;
    bool escaped = false;
};

} // namespace dbms

