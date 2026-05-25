#include <climits>
#include <iostream>
#include <string>

#include "test_utils.hpp"
#include "common/value.hpp"
#include "common/value_comparator.hpp"

using namespace dbms;

void test_compare_int_eq(TestStats& stats) {
    test_header("compare int ==");
    
    check(
        stats,
        ValueComparator::compare(Value(10), Value(10), "=="),
        "10 == 10"
    );
    check(
        stats,
        !ValueComparator::compare(Value(10), Value(20), "=="),
        "10 != 20"
    );
    check(
        stats,
        ValueComparator::compare(Value(0), Value(0), "=="),
        "0 == 0"
    );
    check(
        stats,
        ValueComparator::compare(Value(-5), Value(-5), "=="),
        "-5 == -5"
    );
    check(
        stats,
        ValueComparator::compare(Value(INT_MAX), Value(INT_MAX), "=="),
        "INT_MAX == INT_MAX"
    );
}

void test_compare_int_ne(TestStats& stats) {
    test_header("compare int !=");
    
    check(
        stats,
        ValueComparator::compare(Value(10), Value(20), "!="),
        "10 != 20"
    );
    check(
        stats,
        !ValueComparator::compare(Value(10), Value(10), "!="),
        "10 not != 10"
    );
    check(
        stats,
        ValueComparator::compare(Value(-1), Value(1), "!="),
        "-1 != 1"
    );
    check(
        stats,
        ValueComparator::compare(Value(0), Value(1), "!="),
        "0 != 1"
    );
}

void test_compare_int_lt(TestStats& stats) {
    test_header("compare int <");
    
    check(
        stats,
        ValueComparator::compare(Value(1), Value(10), "<"),
        "1 < 10"
    );
    check(
        stats,
        !ValueComparator::compare(Value(10), Value(1), "<"),
        "10 not < 1"
    );
    check(
        stats,
        !ValueComparator::compare(Value(5), Value(5), "<"),
        "5 not < 5"
    );
    check(
        stats,
        ValueComparator::compare(Value(-10), Value(-1), "<"),
        "-10 < -1"
    );
    check(
        stats,
        ValueComparator::compare(Value(-10), Value(0), "<"),
        "-10 < 0"
    );
    check(stats, ValueComparator::compare(Value(-100), Value(100), "<"),
        "-100 < 100"
    );
}

void test_compare_int_gt(TestStats& stats) {
    test_header("compare int >");
    
    check(
        stats,
        ValueComparator::compare(Value(10), Value(1), ">"),
        "10 > 1"
    );
    check(
        stats,
        !ValueComparator::compare(Value(1), Value(10), ">"),
        "1 not > 10"
    );
    check(
        stats,
        !ValueComparator::compare(Value(5), Value(5), ">"),
        "5 not > 5"
    );
    check(
        stats,
        ValueComparator::compare(Value(0), Value(-10), ">"),
        "0 > -10"
    );
    check(
        stats,
        ValueComparator::compare(Value(100), Value(-100), ">"),
        "100 > -100"
    );
}

void test_compare_int_le(TestStats& stats) {
    test_header("compare int <=");
    
    check(
        stats,
        ValueComparator::compare(Value(1), Value(10), "<="),
        "1 <= 10"
    );
    check(
        stats,
        ValueComparator::compare(Value(5), Value(5), "<="),
        "5 <= 5"
    );
    check(
        stats,
        !ValueComparator::compare(Value(10), Value(1), "<="),
        "10 not <= 1"
    );
    check(
        stats,
        ValueComparator::compare(Value(-10), Value(0), "<="),
        "-10 <= 0"
    );
    check(
        stats,
        ValueComparator::compare(Value(0), Value(0), "<="),
        "0 <= 0"
    );
}

void test_compare_int_ge(TestStats& stats) {
    test_header("compare int >=");
    
    check(
        stats,
        ValueComparator::compare(Value(10), Value(1), ">="),
        "10 >= 1"
    );
    check(
        stats,
        ValueComparator::compare(Value(5), Value(5), ">="),
        "5 >= 5"
    );
    check(
        stats,
        !ValueComparator::compare(Value(1), Value(10), ">="),
        "1 not >= 10"
    );
    check(
        stats,
        ValueComparator::compare(Value(0), Value(-10), ">="),
        "0 >= -10"
    );
}

void test_compare_string_eq(TestStats& stats) {
    test_header("compare string ==");
    
    check(
        stats,
        ValueComparator::compare(Value("hello"), Value("hello"), "=="),
        "hello == hello"
    );
    check(
        stats,
        !ValueComparator::compare(Value("hello"), Value("world"), "=="),
        "hello != world"
    );
    check(
        stats,
        ValueComparator::compare(Value(""), Value(""), "=="),
        "empty == empty"
    );
    check(
        stats,
        ValueComparator::compare(Value("abc"), Value("abc"), "=="),
        "abc == abc"
    );
}

void test_compare_string_ne(TestStats& stats) {
    test_header("compare string !=");
    
    check(
        stats,
        ValueComparator::compare(Value("abc"), Value("xyz"), "!="),
        "abc != xyz"
    );
    check(
        stats,
        !ValueComparator::compare(Value("abc"), Value("abc"), "!="),
        "abc not != abc"
    );
    check(
        stats,
        ValueComparator::compare(Value(""), Value("a"), "!="),
        "empty != a"
    );
}

void test_compare_string_lt(TestStats& stats) {
    test_header("compare string <");
    
    check(
        stats,
        ValueComparator::compare(Value("abc"), Value("xyz"), "<"),
        "abc < xyz"
    );
    check(
        stats,
        !ValueComparator::compare(Value("xyz"), Value("abc"), "<"),
        "xyz not < abc"
    );
    check(
        stats,
        !ValueComparator::compare(Value("abc"), Value("abc"), "<"),
        "abc not < abc"
    );
    check(
        stats,
        ValueComparator::compare(Value(""), Value("a"), "<"),
        "empty < a"
    );
    check(
        stats,
        ValueComparator::compare(Value("apple"), Value("banana"), "<"),
        "apple < banana"
    );
}

void test_compare_string_gt(TestStats& stats) {
    test_header("compare string >");
    
    check(
        stats,
        ValueComparator::compare(Value("xyz"), Value("abc"), ">"),
        "xyz > abc"
    );
    check(
        stats,
        !ValueComparator::compare(Value("abc"), Value("xyz"), ">"),
        "abc not > xyz"
    );
    check(
        stats,
        ValueComparator::compare(Value("banana"), Value("apple"), ">"),
        "banana > apple"
    );
    check(
        stats,
        ValueComparator::compare(Value("a"), Value(""), ">"),
        "a > empty"
    );
}

void test_compare_string_le(TestStats& stats) {
    test_header("compare string <=");
    
    check(
        stats,
        ValueComparator::compare(Value("abc"), Value("xyz"), "<="),
        "abc <= xyz"
    );
    check(
        stats,
        ValueComparator::compare(Value("abc"), Value("abc"), "<="),
        "abc <= abc"
    );
    check(
        stats,
        !ValueComparator::compare(Value("xyz"), Value("abc"), "<="),
        "xyz not <= abc"
    );
    check(
        stats,
        ValueComparator::compare(Value(""), Value(""), "<="),
        "empty <= empty"
    );
}

void test_compare_string_ge(TestStats& stats) {
    test_header("compare string >=");
    
    check(
        stats,
        ValueComparator::compare(Value("xyz"), Value("abc"), ">="),
        "xyz >= abc"
    );
    check(
        stats,
        ValueComparator::compare(Value("abc"), Value("abc"), ">="),
        "abc >= abc"
    );
    check(
        stats,
        !ValueComparator::compare(Value("abc"), Value("xyz"), ">="),
        "abc not >= xyz"
    );
}

void test_compare_null(TestStats& stats) {
    test_header("compare with null");
    
    Value null1;
    Value null2;
    Value int_val(10);
    Value str_val("hello");
    
    check(
        stats,
        ValueComparator::compare(null1, null2, "=="),
        "null == null"
    );
    check(
        stats,
        !ValueComparator::compare(null1, null2, "!="),
        "null not != null"
    );
    check(
        stats,
        !ValueComparator::compare(null1, int_val, "=="),
        "null != int"
    );
    check(
        stats,
        ValueComparator::compare(null1, int_val, "!="),
        "null != int"
    );
    check(
        stats,
        !ValueComparator::compare(null1, int_val, "<"),
        "null not < int"
    );
    check(
        stats,
        !ValueComparator::compare(null1, int_val, ">"),
        "null not > int"
    );
    check(
        stats,
        !ValueComparator::compare(null1, int_val, "<="),
        "null not <= int"
    );
    check(
        stats,
        !ValueComparator::compare(null1, int_val, ">="),
        "null not >= int"
    );
    
    check(
        stats,
        !ValueComparator::compare(int_val, null1, "=="),
        "int != null"
    );
    check(
        stats,
        ValueComparator::compare(int_val, null1, "!="),
        "int != null"
    );
    check(
        stats,
        !ValueComparator::compare(str_val, null1, "=="),
        "string != null"
    );
}

void test_compare_cross_type(TestStats& stats) {
    test_header("compare cross-type");
    
    check(
        stats,
        !ValueComparator::compare(Value(10), Value("10"), "=="),
        "int != string"
    );
    check(
        stats,
        !ValueComparator::compare(Value(10), Value("10"), "!="),
        "int != string"
    );
    check(
        stats,
        !ValueComparator::compare(Value(10), Value("10"), "<"),
        "int not < string"
    );
    check(
        stats,
        !ValueComparator::compare(Value(10), Value("10"), ">"),
        "int not > string"
    );
    check(
        stats,
        !ValueComparator::compare(Value(10), Value("10"), "<="),
        "int not <= string"
    );
    check(
        stats,
        !ValueComparator::compare(Value(10), Value("10"), ">="),
        "int not >= string"
    );
}

void test_compare_unknown_operator(TestStats& stats) {
    test_header("compare unknown operator");
    
    check(
        stats,
        !ValueComparator::compare(Value(10), Value(10), "&&"),
        "Unknown operator returns false"
    );
    check(
        stats,
        !ValueComparator::compare(Value(10), Value(10), "LIKE"),
        "LIKE returns false"
    );
    check(
        stats,
        !ValueComparator::compare(Value(10), Value(10), ""),
        "Empty operator returns false"
    );
    check(
        stats,
        !ValueComparator::compare(Value("abc"), Value("abc"), "BETWEEN"),
        "BETWEEN returns false"
    );
}

void test_between_int(TestStats& stats) {
    test_header("between int");
    
    check(
        stats,
        ValueComparator::between(Value(5), Value(1), Value(10)),
        "5 between 1 and 10"
    );
    check(
        stats,
        !ValueComparator::between(Value(0), Value(1), Value(10)),
        "0 not between 1 and 10"
    );
    check(
        stats,
        !ValueComparator::between(Value(10), Value(1), Value(10)),
        "10 not between 1 and 10 (exclusive upper)"
    );
    check(
        stats,
        ValueComparator::between(Value(1), Value(1), Value(2)),
        "1 between 1 and 2"
    );
    check(
        stats,
        ValueComparator::between(Value(-5), Value(-10), Value(0)),
        "-5 between -10 and 0"
    );
    check(
        stats,
        ValueComparator::between(Value(-10), Value(-10), Value(0)),
        "-10 between -10 and 0 (inclusive lower)"
    );
}

void test_between_string(TestStats& stats) {
    test_header("between string");
    
    check(
        stats,
        ValueComparator::between(Value("banana"), Value("apple"),
        Value("cherry")), "banana between apple and cherry"
    );
    check(
        stats,
        ValueComparator::between(Value("apple"), Value("apple"),
        Value("cherry")), "apple between apple and cherry (inclusive lower)"
    );
    check(
        stats,
        !ValueComparator::between(Value("cherry"), Value("apple"),
        Value("cherry")), "cherry not between apple and cherry"
    );
    check(
        stats,
        !ValueComparator::between(Value("aardvark"), Value("apple"),
        Value("cherry")), "aardvark not between apple and cherry"
    );
    check(
        stats,
        ValueComparator::between(Value("a"), Value("a"), Value("b")), 
        "a between a and b");
}

void test_between_null(TestStats& stats) {
    test_header("between null");
    
    check(
        stats,
        !ValueComparator::between(Value(), Value(1), Value(10)),
        "null not between"
    );
    check(
        stats,
        !ValueComparator::between(Value(5), Value(), Value(10)),
        "null lower bound returns false"
    );
    check(
        stats,
        !ValueComparator::between(Value(5), Value(1), Value()),
        "null upper bound returns false"
    );
    check(
        stats,
        !ValueComparator::between(Value(), Value(), Value()),
        "all null returns false"
    );
}

void test_between_cross_type(TestStats& stats) {
    test_header("between cross-type");
    
    check(
        stats,
        !ValueComparator::between(Value(5), Value("1"), Value("10")),
        "cross-type between returns false"
    );
    check(
        stats,
        !ValueComparator::between(Value("5"), Value(1), Value(10)),
        "cross-type between returns false"
    );
    check(
        stats,
        !ValueComparator::between(Value(5), Value(1), Value("10")),
        "mixed bounds returns false"
    );
}

void test_edge_cases(TestStats& stats) {
    test_header("Edge cases");
    
    check(
        stats,
        ValueComparator::compare(Value(0), Value(0), "=="),
        "0 == 0"
    );
    check(stats, ValueComparator::compare(Value(0), Value(0), "<="), "0 <= 0");
    check(stats, ValueComparator::compare(Value(0), Value(0), ">="), "0 >= 0");
    check(stats, !ValueComparator::compare(Value(0), Value(0), "<"), "0 not < 0");
    check(stats, !ValueComparator::compare(Value(0), Value(0), ">"), "0 not > 0");
    check(
        stats,
        !ValueComparator::compare(Value(0), Value(0), "!="),
        "0 not != 0"
    );
    
    check(
        stats,
        ValueComparator::compare(Value(""), Value(""), "=="),
        "empty == empty"
    );
    check(
        stats,
        ValueComparator::compare(Value(""), Value(""), "<="),
        "empty <= empty"
    );
    check(
        stats,
        ValueComparator::compare(Value(""), Value(""), ">="),
        "empty >= empty"
    );
}

int main() {
    TestStats stats;
    std::cout << "Running ValueComparator tests..." << std::endl;
    
    test_compare_int_eq(stats);
    test_compare_int_ne(stats);
    test_compare_int_lt(stats);
    test_compare_int_gt(stats);
    test_compare_int_le(stats);
    test_compare_int_ge(stats);
    test_compare_string_eq(stats);
    test_compare_string_ne(stats);
    test_compare_string_lt(stats);
    test_compare_string_gt(stats);
    test_compare_string_le(stats);
    test_compare_string_ge(stats);
    test_compare_null(stats);
    test_compare_cross_type(stats);
    test_compare_unknown_operator(stats);
    test_between_int(stats);
    test_between_string(stats);
    test_between_null(stats);
    test_between_cross_type(stats);
    test_edge_cases(stats);
    
    print_test_results(stats);
    if (stats.tests_failed > 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

