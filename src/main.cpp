#include <fstream>
#include <iostream>

#include "io/repl.hpp"

int main(int argc, char** argv) {
    using namespace dbms;

    if (argc == 1) {
        run_interactive_mode();
        return EXIT_SUCCESS;
    }

    if (argc == 2) {
        std::ifstream file(argv[1]);

        if (!file.is_open()) {
            std::cerr << "Failed to open file" << std::endl;
            return EXIT_FAILURE;
        }

        run_batch_mode(file);
        return EXIT_SUCCESS;
    }

    std::cerr << "Usage: ./dbms <script.txt>" << std::endl;
    return EXIT_FAILURE;
}

