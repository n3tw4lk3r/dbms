#pragma once

#include <iostream>
#include <string>

#include "execution/executor.hpp"
#include "sqlparser/parser.hpp"
#include "sqlparser/query_buffer.hpp"

namespace dbms {

bool process_line(
    std::string& line,
    QueryBuffer& buffer,
    Parser& parser,
    Executor& executor
);
void run_interactive_mode();
void run_batch_mode(std::istream& input);

} // namespace dbms

