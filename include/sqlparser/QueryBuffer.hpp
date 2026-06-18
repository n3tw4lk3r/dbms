#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace dbms {

class QueryBuffer {
public:
    std::vector<std::string> Append(std::string_view line);
    bool Empty() const;

private:
    std::string buffer;
    bool in_string = false;
    bool escaped = false;
};

} // namespace dbms

