#pragma once

#include <string_view>

class ScopedDataDirectory {
private:
    bool backup_created = false;

public:
    ScopedDataDirectory();
    ~ScopedDataDirectory();
};

struct TestStats {
    int tests_run = 0;
    int tests_passed = 0;
    int tests_failed = 0;
};

void test_header(std::string_view name);
void check(TestStats& stats, bool condition, std::string_view description);
void print_test_results(const TestStats& stats);

