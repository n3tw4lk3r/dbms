#pragma once

#include <string>

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

void test_header(const std::string& name);
void check(TestStats& stats, bool condition, const std::string& description);
void print_test_results(const TestStats& stats);

