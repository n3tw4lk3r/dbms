#pragma once

#include <iostream>
#include <string>

struct TestStats {
    int tests_run = 0;
    int tests_passed = 0;
    int tests_failed = 0;
};

inline void test_header(const std::string& name) {
    std::cout << "\n=== " << name << " ===" << std::endl;
}

inline void check(TestStats& stats, bool condition, const std::string& description) {
    ++stats.tests_run;
    if (condition) {
        ++stats.tests_passed;
        std::cout << "  [PASS] " << description << std::endl;
    } else {
        ++stats.tests_failed;
        std::cout << "  [FAIL] " << description << std::endl;
    }
}

inline void print_test_results(const TestStats& stats) {
    std::cout << "\n=== Results ===" << std::endl;
    std::cout << "Tests run: " << stats.tests_run << std::endl;
    std::cout << "Passed: " << stats.tests_passed << std::endl;
    std::cout << "Failed: " << stats.tests_failed << std::endl;
}

