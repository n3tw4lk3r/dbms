#include <exception>
#include <fstream>
#include <iostream>
#include <string>

#include "catalog/system.hpp"
#include "execution/executor.hpp"
#include "sqlparser/parser.hpp"
#include "sqlparser/query_buffer.hpp"

namespace dbms {

void executeQueries(
    std::istream& input,
    Parser& parser,
    Executor& executor
) {
    QueryBuffer buffer;
    std::string line;

    while (std::getline(input, line)) {
        if (line == "exit") {
            break;
        }

        auto queries = buffer.append(line);
        for (const auto& query : queries) {
            try {
                auto cmd = parser.parse(query);
                executor.execute(cmd);

            } catch (const std::exception& error) {
                std::cerr << error.what() << std::endl;
            }
        }
    }
}

} // namespace dbms

int main(int argc, char** argv) {
    using namespace dbms;

    System system;
    Parser parser;
    Executor executor(system);

    if (argc == 1) {
        std::cout << "Enter 'exit' to exit." << std::endl;
        executeQueries(std::cin, parser, executor);
        return EXIT_SUCCESS;
    }

    if (argc == 2) {
        std::ifstream file(argv[1]);

        if (!file.is_open()) {
            std::cerr << "Failed to open file" << std::endl;
            return EXIT_FAILURE;
        }

        executeQueries(file, parser, executor);
        return EXIT_SUCCESS;
    }

    std::cerr << "Usage: ./dbms <script.txt>" << std::endl;
    return EXIT_FAILURE;
}

