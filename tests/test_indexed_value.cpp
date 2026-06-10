#include <iostream>
#include <string>

#include "common/value.hpp"
#include "storage/indexed_value.hpp"
#include "utils.hpp"

using namespace dbms;

void test_indexed_value_default_constructor(TestStats& stats) {
    test_header("Default constructor");
    IndexedValue iv;
    check(stats, iv.GetValue().IsNull(), "Default indexed value is null");
}

void test_indexed_value_value_constructor(TestStats& stats) {
    test_header("Value constructor");
    
    IndexedValue iv_int(Value(42));
    check(
        stats,
        iv_int.GetValue().GetType() == Value::Type::kInt,
        "Int type preserved"
    );
    check(stats, iv_int.GetValue().AsInt() == 42, "Int value preserved");
    
    IndexedValue iv_str(Value("hello"));
    check(
        stats,
        iv_str.GetValue().GetType() == Value::Type::kString,
        "String type preserved"
    );
    check(
        stats,
        iv_str.GetValue().AsString() == "hello",
        "String value preserved"
    );
    
    IndexedValue iv_null;
    check(stats, iv_null.GetValue().IsNull(), "Null value preserved");
}

void test_indexed_value_int_comparison_less(TestStats& stats) {
    test_header("Int comparison less");
    
    IndexedValue small(Value(1));
    IndexedValue medium(Value(10));
    IndexedValue large(Value(100));
    IndexedValue also_small(Value(1));
    IndexedValue neg(Value(-5));
    IndexedValue zero(Value(0));
    
    check(stats, small < medium, "1 < 10");
    check(stats, medium < large, "10 < 100");
    check(stats, !(medium < small), "10 not < 1");
    check(stats, !(small < also_small), "1 not < 1");
    check(stats, !(also_small < small), "1 not < 1 (reverse)");
    check(stats, neg < zero, "-5 < 0");
    check(stats, zero < small, "0 < 1");
    check(stats, neg < small, "-5 < 1");
}

void test_indexed_value_int_comparison_greater(TestStats& stats) {
    test_header("Int comparison greater");
    
    IndexedValue small(Value(1));
    IndexedValue medium(Value(10));
    IndexedValue large(Value(100));
    IndexedValue also_small(Value(1));
    IndexedValue neg(Value(-5));
    IndexedValue zero(Value(0));
    
    check(stats, large > medium, "100 > 10");
    check(stats, medium > small, "10 > 1");
    check(stats, !(small > medium), "1 not > 10");
    check(stats, !(small > also_small), "1 not > 1");
    check(stats, zero > neg, "0 > -5");
    check(stats, large > neg, "100 > -5");
}

void test_indexed_value_int_comparison_equal(TestStats& stats) {
    test_header("Int comparison equal");
    
    IndexedValue one(Value(1));
    IndexedValue also_one(Value(1));
    IndexedValue two(Value(2));
    IndexedValue neg_one(Value(-1));
    IndexedValue also_neg_one(Value(-1));
    
    check(stats, one == also_one, "1 == 1");
    check(stats, !(one == two), "1 != 2");
    check(stats, neg_one == also_neg_one, "-1 == -1");
    check(stats, !(one == neg_one), "1 != -1");
}

void test_indexed_value_string_comparison_less(TestStats& stats) {
    test_header("String comparison less");
    
    IndexedValue apple(Value("apple"));
    IndexedValue banana(Value("banana"));
    IndexedValue empty(Value(""));
    IndexedValue upper(Value("APPLE"));
    
    check(stats, apple < banana, "apple < banana");
    check(stats, !(banana < apple), "banana not < apple");
    check(stats, empty < apple, "empty < apple");
    check(stats, !(apple < empty), "apple not < empty");
    check(stats, upper < apple, "APPLE < apple (ASCII order)");
}

void test_indexed_value_string_comparison_greater(TestStats& stats) {
    test_header("String comparison greater");
    
    IndexedValue apple(Value("apple"));
    IndexedValue banana(Value("banana"));
    IndexedValue cherry(Value("cherry"));
    
    check(stats, banana > apple, "banana > apple");
    check(stats, cherry > banana, "cherry > banana");
    check(stats, !(apple > banana), "apple not > banana");
    check(stats, !(apple > cherry), "apple not > cherry");
}

void test_indexed_value_string_comparison_equal(TestStats& stats) {
    test_header("String comparison equal");
    
    IndexedValue hello(Value("hello"));
    IndexedValue also_hello(Value("hello"));
    IndexedValue world(Value("world"));
    IndexedValue empty1(Value(""));
    IndexedValue empty2(Value(""));
    
    check(stats, hello == also_hello, "hello == hello");
    check(stats, !(hello == world), "hello != world");
    check(stats, empty1 == empty2, "empty == empty");
}

void test_indexed_value_cross_type_comparison(TestStats& stats) {
    test_header("Cross-type comparison");
    
    IndexedValue iv_int(Value(42));
    IndexedValue iv_str(Value("42"));
    IndexedValue iv_null;
    
    check(stats, !(iv_int < iv_str), "int not < string");
    check(stats, !(iv_str < iv_int), "string not < int");
    check(stats, !(iv_int > iv_str), "int not > string");
    check(stats, !(iv_int == iv_str), "int != string");
    check(stats, !(iv_int == iv_null), "int != null");
    check(stats, !(iv_null == iv_str), "null != string");
}

void test_indexed_value_null_comparison(TestStats& stats) {
    test_header("Null comparison");
    
    IndexedValue null1;
    IndexedValue null2;
    IndexedValue int_val(Value(0));
    
    check(stats, !(null1 == null2), "null != null (both null)");
}

void test_indexed_value_transitivity(TestStats& stats) {
    test_header("Transitivity of comparison");
    
    IndexedValue a(Value(1));
    IndexedValue b(Value(2));
    IndexedValue c(Value(3));
    IndexedValue d(Value(4));
    IndexedValue e(Value(5));
    
    bool chain = a < b && b < c && c < d && d < e;
    check(stats, chain, "Chain 1 < 2 < 3 < 4 < 5 holds");
    if (chain) {
        check(stats, a < e, "Transitivity: 1 < 5 via chain");
        check(stats, a < c, "Transitivity: 1 < 3 via chain");
        check(stats, b < e, "Transitivity: 2 < 5 via chain");
    }
}

void test_indexed_value_get_value_returns_reference(TestStats& stats) {
    test_header("getValue returns correct reference");
    
    Value v(42);
    IndexedValue iv(v);
    const Value& ref = iv.GetValue();
    check(stats, ref.GetType() == Value::Type::kInt, "Reference type is Int");
    check(stats, ref.AsInt() == 42, "Reference value is 42");
    
    Value v_str("test");
    IndexedValue iv_str(v_str);
    const Value& ref_str = iv_str.GetValue();
    check(
        stats,
        ref_str.GetType() == Value::Type::kString,
        "String reference type"
    );
    check(stats, ref_str.AsString() == "test", "String reference value");
}

void test_indexed_value_negative_numbers_comparison(TestStats& stats) {
    test_header("Negative numbers comparison");
    
    IndexedValue neg100(Value(-100));
    IndexedValue neg50(Value(-50));
    IndexedValue neg1(Value(-1));
    IndexedValue zero(Value(0));
    IndexedValue pos1(Value(1));
    
    check(stats, neg100 < neg50, "-100 < -50");
    check(stats, neg50 < neg1, "-50 < -1");
    check(stats, neg1 < zero, "-1 < 0");
    check(stats, zero < pos1, "0 < 1");
    check(stats, neg100 < pos1, "-100 < 1");
    check(stats, neg50 > neg100, "-50 > -100");
    check(stats, zero > neg1, "0 > -1");
    check(stats, !(neg50 == neg100), "-50 != -100");
    check(stats, neg50 == IndexedValue(Value(-50)), "-50 == -50");
}

void test_indexed_value_large_comparisons(TestStats& stats) {
    test_header("Large value comparisons");
    
    IndexedValue large1(Value(1000000));
    IndexedValue large2(Value(2000000));
    IndexedValue large3(Value(1000000));
    
    check(stats, large1 < large2, "1000000 < 2000000");
    check(stats, large2 > large1, "2000000 > 1000000");
    check(stats, large1 == large3, "1000000 == 1000000");
    check(stats, !(large1 == large2), "1000000 != 2000000");
}

int main() {
    TestStats stats;
    std::cout << "Running IndexedValue tests..." << std::endl;
    
    test_indexed_value_default_constructor(stats);
    test_indexed_value_value_constructor(stats);
    test_indexed_value_int_comparison_less(stats);
    test_indexed_value_int_comparison_greater(stats);
    test_indexed_value_int_comparison_equal(stats);
    test_indexed_value_string_comparison_less(stats);
    test_indexed_value_string_comparison_greater(stats);
    test_indexed_value_string_comparison_equal(stats);
    test_indexed_value_cross_type_comparison(stats);
    test_indexed_value_null_comparison(stats);
    test_indexed_value_transitivity(stats);
    test_indexed_value_get_value_returns_reference(stats);
    test_indexed_value_negative_numbers_comparison(stats);
    test_indexed_value_large_comparisons(stats);
    
    print_test_results(stats);
    if (stats.tests_failed > 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

