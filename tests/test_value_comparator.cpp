#include <climits>
#include <iostream>
#include <string>

#include "common/value.hpp"
#include "common/value_comparator.hpp"
#include "utils.hpp"

using namespace dbms;

void test_value_comparator_compare_int_eq(TestStats& stats) {
    test_header("compare int ==");
    
    check(
        stats,
        ValueComparator::Compare(Value(10), Value(10), "=="),
        "10 == 10"
    );
    check(
        stats,
        !ValueComparator::Compare(Value(10), Value(20), "=="),
        "10 != 20"
    );
    check(
        stats,
        ValueComparator::Compare(Value(0), Value(0), "=="),
        "0 == 0"
    );
    check(
        stats,
        ValueComparator::Compare(Value(-5), Value(-5), "=="),
        "-5 == -5"
    );
    check(
        stats,
        ValueComparator::Compare(Value(INT_MAX), Value(INT_MAX), "=="),
        "INT_MAX == INT_MAX"
    );
}

void test_value_comparator_compare_int_ne(TestStats& stats) {
    test_header("compare int !=");
    
    check(
        stats,
        ValueComparator::Compare(Value(10), Value(20), "!="),
        "10 != 20"
    );
    check(
        stats,
        !ValueComparator::Compare(Value(10), Value(10), "!="),
        "10 not != 10"
    );
    check(
        stats,
        ValueComparator::Compare(Value(-1), Value(1), "!="),
        "-1 != 1"
    );
    check(
        stats,
        ValueComparator::Compare(Value(0), Value(1), "!="),
        "0 != 1"
    );
}

void test_value_comparator_compare_int_lt(TestStats& stats) {
    test_header("compare int <");
    
    check(
        stats,
        ValueComparator::Compare(Value(1), Value(10), "<"),
        "1 < 10"
    );
    check(
        stats,
        !ValueComparator::Compare(Value(10), Value(1), "<"),
        "10 not < 1"
    );
    check(
        stats,
        !ValueComparator::Compare(Value(5), Value(5), "<"),
        "5 not < 5"
    );
    check(
        stats,
        ValueComparator::Compare(Value(-10), Value(-1), "<"),
        "-10 < -1"
    );
    check(
        stats,
        ValueComparator::Compare(Value(-10), Value(0), "<"),
        "-10 < 0"
    );
    check(stats, ValueComparator::Compare(Value(-100), Value(100), "<"),
        "-100 < 100"
    );
}

void test_value_comparator_compare_int_gt(TestStats& stats) {
    test_header("compare int >");
    
    check(
        stats,
        ValueComparator::Compare(Value(10), Value(1), ">"),
        "10 > 1"
    );
    check(
        stats,
        !ValueComparator::Compare(Value(1), Value(10), ">"),
        "1 not > 10"
    );
    check(
        stats,
        !ValueComparator::Compare(Value(5), Value(5), ">"),
        "5 not > 5"
    );
    check(
        stats,
        ValueComparator::Compare(Value(0), Value(-10), ">"),
        "0 > -10"
    );
    check(
        stats,
        ValueComparator::Compare(Value(100), Value(-100), ">"),
        "100 > -100"
    );
}

void test_value_comparator_compare_int_le(TestStats& stats) {
    test_header("compare int <=");
    
    check(
        stats,
        ValueComparator::Compare(Value(1), Value(10), "<="),
        "1 <= 10"
    );
    check(
        stats,
        ValueComparator::Compare(Value(5), Value(5), "<="),
        "5 <= 5"
    );
    check(
        stats,
        !ValueComparator::Compare(Value(10), Value(1), "<="),
        "10 not <= 1"
    );
    check(
        stats,
        ValueComparator::Compare(Value(-10), Value(0), "<="),
        "-10 <= 0"
    );
    check(
        stats,
        ValueComparator::Compare(Value(0), Value(0), "<="),
        "0 <= 0"
    );
}

void test_value_comparator_compare_int_ge(TestStats& stats) {
    test_header("compare int >=");
    
    check(
        stats,
        ValueComparator::Compare(Value(10), Value(1), ">="),
        "10 >= 1"
    );
    check(
        stats,
        ValueComparator::Compare(Value(5), Value(5), ">="),
        "5 >= 5"
    );
    check(
        stats,
        !ValueComparator::Compare(Value(1), Value(10), ">="),
        "1 not >= 10"
    );
    check(
        stats,
        ValueComparator::Compare(Value(0), Value(-10), ">="),
        "0 >= -10"
    );
}

void test_value_comparator_compare_string_eq(TestStats& stats) {
    test_header("compare string ==");
    
    check(
        stats,
        ValueComparator::Compare(Value("hello"), Value("hello"), "=="),
        "hello == hello"
    );
    check(
        stats,
        !ValueComparator::Compare(Value("hello"), Value("world"), "=="),
        "hello != world"
    );
    check(
        stats,
        ValueComparator::Compare(Value(""), Value(""), "=="),
        "empty == empty"
    );
    check(
        stats,
        ValueComparator::Compare(Value("abc"), Value("abc"), "=="),
        "abc == abc"
    );
}

void test_value_comparator_compare_string_ne(TestStats& stats) {
    test_header("compare string !=");
    
    check(
        stats,
        ValueComparator::Compare(Value("abc"), Value("xyz"), "!="),
        "abc != xyz"
    );
    check(
        stats,
        !ValueComparator::Compare(Value("abc"), Value("abc"), "!="),
        "abc not != abc"
    );
    check(
        stats,
        ValueComparator::Compare(Value(""), Value("a"), "!="),
        "empty != a"
    );
}

void test_value_comparator_compare_string_lt(TestStats& stats) {
    test_header("compare string <");
    
    check(
        stats,
        ValueComparator::Compare(Value("abc"), Value("xyz"), "<"),
        "abc < xyz"
    );
    check(
        stats,
        !ValueComparator::Compare(Value("xyz"), Value("abc"), "<"),
        "xyz not < abc"
    );
    check(
        stats,
        !ValueComparator::Compare(Value("abc"), Value("abc"), "<"),
        "abc not < abc"
    );
    check(
        stats,
        ValueComparator::Compare(Value(""), Value("a"), "<"),
        "empty < a"
    );
    check(
        stats,
        ValueComparator::Compare(Value("apple"), Value("banana"), "<"),
        "apple < banana"
    );
}

void test_value_comparator_compare_string_gt(TestStats& stats) {
    test_header("compare string >");
    
    check(
        stats,
        ValueComparator::Compare(Value("xyz"), Value("abc"), ">"),
        "xyz > abc"
    );
    check(
        stats,
        !ValueComparator::Compare(Value("abc"), Value("xyz"), ">"),
        "abc not > xyz"
    );
    check(
        stats,
        ValueComparator::Compare(Value("banana"), Value("apple"), ">"),
        "banana > apple"
    );
    check(
        stats,
        ValueComparator::Compare(Value("a"), Value(""), ">"),
        "a > empty"
    );
}

void test_value_comparator_compare_string_le(TestStats& stats) {
    test_header("compare string <=");
    
    check(
        stats,
        ValueComparator::Compare(Value("abc"), Value("xyz"), "<="),
        "abc <= xyz"
    );
    check(
        stats,
        ValueComparator::Compare(Value("abc"), Value("abc"), "<="),
        "abc <= abc"
    );
    check(
        stats,
        !ValueComparator::Compare(Value("xyz"), Value("abc"), "<="),
        "xyz not <= abc"
    );
    check(
        stats,
        ValueComparator::Compare(Value(""), Value(""), "<="),
        "empty <= empty"
    );
}

void test_value_comparator_compare_string_ge(TestStats& stats) {
    test_header("compare string >=");
    
    check(
        stats,
        ValueComparator::Compare(Value("xyz"), Value("abc"), ">="),
        "xyz >= abc"
    );
    check(
        stats,
        ValueComparator::Compare(Value("abc"), Value("abc"), ">="),
        "abc >= abc"
    );
    check(
        stats,
        !ValueComparator::Compare(Value("abc"), Value("xyz"), ">="),
        "abc not >= xyz"
    );
}

void test_value_comparator_compare_null(TestStats& stats) {
    test_header("compare with null");
    
    Value null1;
    Value null2;
    Value int_val(10);
    Value str_val("hello");
    
    check(
        stats,
        ValueComparator::Compare(null1, null2, "=="),
        "null == null"
    );
    check(
        stats,
        !ValueComparator::Compare(null1, null2, "!="),
        "null not != null"
    );
    check(
        stats,
        !ValueComparator::Compare(null1, int_val, "=="),
        "null != int"
    );
    check(
        stats,
        ValueComparator::Compare(null1, int_val, "!="),
        "null != int"
    );
    check(
        stats,
        !ValueComparator::Compare(null1, int_val, "<"),
        "null not < int"
    );
    check(
        stats,
        !ValueComparator::Compare(null1, int_val, ">"),
        "null not > int"
    );
    check(
        stats,
        !ValueComparator::Compare(null1, int_val, "<="),
        "null not <= int"
    );
    check(
        stats,
        !ValueComparator::Compare(null1, int_val, ">="),
        "null not >= int"
    );
    
    check(
        stats,
        !ValueComparator::Compare(int_val, null1, "=="),
        "int != null"
    );
    check(
        stats,
        ValueComparator::Compare(int_val, null1, "!="),
        "int != null"
    );
    check(
        stats,
        !ValueComparator::Compare(str_val, null1, "=="),
        "string != null"
    );
}

void test_value_comparator_compare_cross_type(TestStats& stats) {
    test_header("compare cross-type");
    
    check(
        stats,
        !ValueComparator::Compare(Value(10), Value("10"), "=="),
        "int != string"
    );
    check(
        stats,
        !ValueComparator::Compare(Value(10), Value("10"), "!="),
        "int != string"
    );
    check(
        stats,
        !ValueComparator::Compare(Value(10), Value("10"), "<"),
        "int not < string"
    );
    check(
        stats,
        !ValueComparator::Compare(Value(10), Value("10"), ">"),
        "int not > string"
    );
    check(
        stats,
        !ValueComparator::Compare(Value(10), Value("10"), "<="),
        "int not <= string"
    );
    check(
        stats,
        !ValueComparator::Compare(Value(10), Value("10"), ">="),
        "int not >= string"
    );
}

void test_value_comparator_compare_unknown_operator(TestStats& stats) {
    test_header("compare unknown operator");
    
    check(
        stats,
        !ValueComparator::Compare(Value(10), Value(10), "&&"),
        "Unknown operator returns false"
    );
    check(
        stats,
        !ValueComparator::Compare(Value(10), Value(10), "LIKE"),
        "LIKE returns false"
    );
    check(
        stats,
        !ValueComparator::Compare(Value(10), Value(10), ""),
        "Empty operator returns false"
    );
    check(
        stats,
        !ValueComparator::Compare(Value("abc"), Value("abc"), "BETWEEN"),
        "BETWEEN returns false"
    );
}

void test_value_comparator_between_int(TestStats& stats) {
    test_header("between int");
    
    check(
        stats,
        ValueComparator::Between(Value(5), Value(1), Value(10)),
        "5 between 1 and 10"
    );
    check(
        stats,
        !ValueComparator::Between(Value(0), Value(1), Value(10)),
        "0 not between 1 and 10"
    );
    check(
        stats,
        !ValueComparator::Between(Value(10), Value(1), Value(10)),
        "10 not between 1 and 10 (exclusive upper)"
    );
    check(
        stats,
        ValueComparator::Between(Value(1), Value(1), Value(2)),
        "1 between 1 and 2"
    );
    check(
        stats,
        ValueComparator::Between(Value(-5), Value(-10), Value(0)),
        "-5 between -10 and 0"
    );
    check(
        stats,
        ValueComparator::Between(Value(-10), Value(-10), Value(0)),
        "-10 between -10 and 0 (inclusive lower)"
    );
}

void test_value_comparator_between_string(TestStats& stats) {
    test_header("between string");
    
    check(
        stats,
        ValueComparator::Between(Value("banana"), Value("apple"),
        Value("cherry")), "banana between apple and cherry"
    );
    check(
        stats,
        ValueComparator::Between(Value("apple"), Value("apple"),
        Value("cherry")), "apple between apple and cherry (inclusive lower)"
    );
    check(
        stats,
        !ValueComparator::Between(Value("cherry"), Value("apple"),
        Value("cherry")), "cherry not between apple and cherry"
    );
    check(
        stats,
        !ValueComparator::Between(Value("aardvark"), Value("apple"),
        Value("cherry")), "aardvark not between apple and cherry"
    );
    check(
        stats,
        ValueComparator::Between(Value("a"), Value("a"), Value("b")), 
        "a between a and b");
}

void test_value_comparator_between_null(TestStats& stats) {
    test_header("between null");
    
    check(
        stats,
        !ValueComparator::Between(Value(), Value(1), Value(10)),
        "null not between"
    );
    check(
        stats,
        !ValueComparator::Between(Value(5), Value(), Value(10)),
        "null lower bound returns false"
    );
    check(
        stats,
        !ValueComparator::Between(Value(5), Value(1), Value()),
        "null upper bound returns false"
    );
    check(
        stats,
        !ValueComparator::Between(Value(), Value(), Value()),
        "all null returns false"
    );
}

void test_value_comparator_between_cross_type(TestStats& stats) {
    test_header("between cross-type");
    
    check(
        stats,
        !ValueComparator::Between(Value(5), Value("1"), Value("10")),
        "cross-type between returns false"
    );
    check(
        stats,
        !ValueComparator::Between(Value("5"), Value(1), Value(10)),
        "cross-type between returns false"
    );
    check(
        stats,
        !ValueComparator::Between(Value(5), Value(1), Value("10")),
        "mixed bounds returns false"
    );
}

void test_value_comparator_edge_cases(TestStats& stats) {
    test_header("Edge cases");
    
    check(
        stats,
        ValueComparator::Compare(Value(0), Value(0), "=="),
        "0 == 0"
    );
    check(stats, ValueComparator::Compare(Value(0), Value(0), "<="), "0 <= 0");
    check(stats, ValueComparator::Compare(Value(0), Value(0), ">="), "0 >= 0");
    check(stats, !ValueComparator::Compare(Value(0), Value(0), "<"), "0 not < 0");
    check(stats, !ValueComparator::Compare(Value(0), Value(0), ">"), "0 not > 0");
    check(
        stats,
        !ValueComparator::Compare(Value(0), Value(0), "!="),
        "0 not != 0"
    );
    
    check(
        stats,
        ValueComparator::Compare(Value(""), Value(""), "=="),
        "empty == empty"
    );
    check(
        stats,
        ValueComparator::Compare(Value(""), Value(""), "<="),
        "empty <= empty"
    );
    check(
        stats,
        ValueComparator::Compare(Value(""), Value(""), ">="),
        "empty >= empty"
    );
}

int main() {
    TestStats stats;
    std::cout << "Running ValueComparator tests..." << std::endl;
    
    test_value_comparator_compare_int_eq(stats);
    test_value_comparator_compare_int_ne(stats);
    test_value_comparator_compare_int_lt(stats);
    test_value_comparator_compare_int_gt(stats);
    test_value_comparator_compare_int_le(stats);
    test_value_comparator_compare_int_ge(stats);
    test_value_comparator_compare_string_eq(stats);
    test_value_comparator_compare_string_ne(stats);
    test_value_comparator_compare_string_lt(stats);
    test_value_comparator_compare_string_gt(stats);
    test_value_comparator_compare_string_le(stats);
    test_value_comparator_compare_string_ge(stats);
    test_value_comparator_compare_null(stats);
    test_value_comparator_compare_cross_type(stats);
    test_value_comparator_compare_unknown_operator(stats);
    test_value_comparator_between_int(stats);
    test_value_comparator_between_string(stats);
    test_value_comparator_between_null(stats);
    test_value_comparator_between_cross_type(stats);
    test_value_comparator_edge_cases(stats);
    
    print_test_results(stats);
    if (stats.tests_failed > 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

