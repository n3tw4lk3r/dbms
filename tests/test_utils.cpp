#include "test_utils.hpp"

#include <filesystem>
#include <iostream>

ScopedDataDirectory::ScopedDataDirectory() {
    const std::filesystem::path data_path = "data";
    const std::filesystem::path backup_path =
        "data_backup_test";

    if (std::filesystem::exists(backup_path)) {
        std::filesystem::remove_all(backup_path);
    }

    if (std::filesystem::exists(data_path)) {
        std::filesystem::rename(
            data_path,
            backup_path
        );

        backup_created = true;
    }
}

ScopedDataDirectory::~ScopedDataDirectory() {
    const std::filesystem::path data_path = "data";
    const std::filesystem::path backup_path = "data_backup_test";

    if (std::filesystem::exists(data_path)) {
        std::filesystem::remove_all(data_path);
    }

    if (
        backup_created &&
        std::filesystem::exists(backup_path)
    ) {
        std::filesystem::rename(
            backup_path,
            data_path
        );
    }
}

void test_header(const std::string& name) {
    std::cout << "\n=== " << name << " ===" << std::endl;
}

void check(TestStats& stats, bool condition, const std::string& description) {
    ++stats.tests_run;
    if (condition) {
        ++stats.tests_passed;
        std::cout << "  [PASS] " << description << std::endl;
    } else {
        ++stats.tests_failed;
        std::cout << "  [FAIL] " << description << std::endl;
    }
}

void print_test_results(const TestStats& stats) {
    std::cout << "\n=== Results ===" << std::endl;
    std::cout << "Tests run: " << stats.tests_run << std::endl;
    std::cout << "Passed: " << stats.tests_passed << std::endl;
    std::cout << "Failed: " << stats.tests_failed << std::endl;
}

