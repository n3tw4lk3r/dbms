#pragma once

#include <iostream>
#include <string>

#include "execution/Executor.hpp"
#include "sqlparser/Parser.hpp"
#include "sqlparser/QueryBuffer.hpp"

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

