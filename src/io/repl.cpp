#include "io/repl.hpp"

#include <exception>
#include <iostream>
#include <string>

#include "catalog/system.hpp"
#include "execution/executor.hpp"
#include "sqlparser/parser.hpp"
#include "sqlparser/query_buffer.hpp"

namespace dbms {

bool process_line(
    std::string& line,
    QueryBuffer& buffer,
    Parser& parser,
    Executor& executor
) {
    if (line == "exit" || line == "exit;") {
        return false;
    }

    auto queries = buffer.append(line);
    for (const auto& query : queries) {
        try {
            auto cmd = parser.parse(query);
            executor.execute(cmd);
        } catch (const std::exception& error) {
            std::cerr << error.what() << std::endl;
            return true;
        }
    }
    return true;
}

void run_interactive_mode() {
    std::cout << "Enter 'exit' to exit." << std::endl;
    
    System system;
    Parser parser;
    Executor executor(system);
    QueryBuffer buffer;

    bool should_exit = false;
    bool expecting_command_end = false;
    while (!should_exit) {
        if (!expecting_command_end) {
            std::cout << "> ";
        }
        
        std::string line;
        std::getline(std::cin, line); 
        
        if (line.empty()) {
            continue;
        }

        if (line.substr(line.size() - 1) != ";") {
            expecting_command_end = true;
        } else {
            expecting_command_end = false;
        }
        
        if (!process_line(line, buffer, parser, executor)) {
            should_exit = true;
            break;
        }
    }

}

void run_batch_mode(std::istream& input) {
    System system;
    Parser parser;
    Executor executor(system);
    QueryBuffer buffer;

    bool should_exit = false;
    while (!should_exit) {
        std::string line;
        std::getline(input, line);

        if (!process_line(line, buffer, parser, executor)) {
            should_exit = true;
            break;
        }
    }
}

} // namespace dbms

