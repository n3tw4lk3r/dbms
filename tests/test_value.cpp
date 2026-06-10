#include <climits>
#include <iostream>
#include <string>

#include "common/value.hpp"
#include "utils.hpp"

using namespace dbms;

void test_value_default_constructor(TestStats& stats) {
    test_header("Default constructor");
    Value v;
    check(stats, v.IsNull(), "Default constructed value is null");
    check(stats, v.GetType() == Value::Type::kNull, "Default type is kNull");
}

void test_value_int_constructor(TestStats& stats) {
    test_header("Int constructor");
    
    Value v0(0);
    check(stats, !v0.IsNull(), "Zero int is not null");
    check(stats, v0.GetType() == Value::Type::kInt, "Type is kInt for zero");
    check(stats, v0.AsInt() == 0, "asInt returns 0");
    
    Value v1(42);
    check(stats, v1.GetType() == Value::Type::kInt, "Type is kInt");
    check(stats, v1.AsInt() == 42, "asInt returns 42");
    
    Value v_neg(-100);
    check(
        stats,
        v_neg.GetType() == Value::Type::kInt,
        "Negative int type is kInt"
    );
    check(stats, v_neg.AsInt() == -100, "Negative int value preserved");
    
    Value v_max(INT_MAX);
    check(stats, v_max.AsInt() == INT_MAX, "INT_MAX preserved");
    
    Value v_min(INT_MIN);
    check(stats, v_min.AsInt() == INT_MIN, "INT_MIN preserved");

}

void test_value_string_constructor(TestStats& stats) {
    test_header("String constructor");
    
    Value v_empty("");
    check(stats, !v_empty.IsNull(), "Empty string is not null");
    check(
        stats,
        v_empty.GetType() == Value::Type::kString,
        "Empty string type is kString"
    );
    check(stats, v_empty.AsString().empty(), "Empty string is empty");
    
    Value v1("hello");
    check(stats, v1.GetType() == Value::Type::kString, "Type is kString");
    check(stats, v1.AsString() == "hello", "String value preserved");
    
    Value v_special("!@#$%^&*()_+-=[]{}|;':\",./<>?");
    check(
        stats,
        v_special.AsString() == "!@#$%^&*()_+-=[]{}|;':\",./<>?",
        "Special chars preserved"
    );
    
    Value v_long(std::string(1000, 'x'));
    check(
        stats,
        v_long.AsString().size() == 1000,
        "Long string length preserved"
    );
    
    Value v_unicode("привет мир 世界");
    check(stats, v_unicode.AsString() == "привет мир 世界", "Unicode preserved");
}

void test_value_null_value_properties(TestStats& stats) {
    test_header("Null value properties");
    Value v;
    check(stats, v.IsNull(), "isNull true for null");
    check(stats, v.GetType() == Value::Type::kNull, "Type is kNull");
    
    Value v_int(10);
    check(stats, !v_int.IsNull(), "Int is not null");
    
    Value v_str("text");
    check(stats, !v_str.IsNull(), "String is not null");
    
    Value v_zero(0);
    check(stats, !v_zero.IsNull(), "Zero int is not null");
    
    Value v_empty_str("");
    check(stats, !v_empty_str.IsNull(), "Empty string is not null");
}

void test_value_type_consistency(TestStats& stats) {
    test_header("Type consistency");
    
    Value v_int(100);
    check(stats, v_int.GetType() == Value::Type::kInt, "Int type consistent");
    check(stats, v_int.AsInt() == 100, "Int value consistent on first call");
    check(stats, v_int.AsInt() == 100, "Int value consistent on second call");
    
    Value v_str("test");
    check(
        stats,
        v_str.GetType() == Value::Type::kString,
        "String type consistent"
    );
    check(
        stats,
        v_str.AsString() == "test",
        "String value consistent on first call"
    );
    check(
        stats,
        v_str.AsString() == "test",
        "String value consistent on second call"
    );
}

void test_value_copy(TestStats& stats) {
    test_header("Value copy semantics");
    
    Value original_int(42);
    Value copy_int = original_int;
    check(
        stats,
        copy_int.GetType() == Value::Type::kInt,
        "Copied int type preserved"
    );
    check(stats, copy_int.AsInt() == 42, "Copied int value preserved");
    
    Value original_str("original");
    Value copy_str = original_str;
    check(
        stats,
        copy_str.GetType() == Value::Type::kString,
        "Copied string type preserved"
    );
    check(
        stats,
        copy_str.AsString() == "original",
        "Copied string value preserved"
    );
    
    Value original_null;
    Value copy_null = original_null;
    check(stats, copy_null.IsNull(), "Copied null is null");
}

void test_value_assignment(TestStats& stats) {
    test_header("Value assignment");
    
    Value v1(10);
    Value v2("hello");
    Value v3;
    
    v1 = v2;
    check(
        stats,
        v1.GetType() == Value::Type::kString,
        "Assignment changes type"
    );
    check(stats, v1.AsString() == "hello", "Assignment changes value");
    
    v2 = v3;
    check(stats, v2.IsNull(), "Assignment to null works");
    
    v1 = Value(99);
    check(
        stats,
        v1.GetType() == Value::Type::kInt,
        "Assignment from temporary int works"
    );
    check(stats, v1.AsInt() == 99, "Temporary int value preserved");
    
    v1 = Value("temp");
    check(stats, v1.GetType() == Value::Type::kString,
        "Assignment from temporary string works"
    );
    check(stats, v1.AsString() == "temp", "Temporary string value preserved");
}

void test_value_boundary_values(TestStats& stats) {
    test_header("Boundary values");
    
    Value zero_int(0);
    check(stats, !zero_int.IsNull(), "Zero is not null");
    check(stats, zero_int.GetType() == Value::Type::kInt, "Zero type is int");
    check(stats, zero_int.AsInt() == 0, "Zero asInt is 0");
    
    Value large(999999999);
    check(stats, large.AsInt() == 999999999, "Large int preserved");
    
    Value very_large(INT_MAX);
    check(stats, very_large.AsInt() == INT_MAX, "INT_MAX preserved");
    
    Value very_small(INT_MIN);
    check(stats, very_small.AsInt() == INT_MIN, "INT_MIN preserved");
}

void test_value_multiple_instances(TestStats& stats) {
    test_header("Multiple instances independence");
    
    Value a(10);
    Value b(20);
    Value c("hello");
    Value d;
    
    check(stats, a.AsInt() == 10, "a is independent");
    check(stats, b.AsInt() == 20, "b is independent");
    check(stats, c.AsString() == "hello", "c is independent");
    check(stats, d.IsNull(), "d is independent");
    
    a = Value(30);
    check(stats, a.AsInt() == 30, "a changed to 30");
    check(stats, b.AsInt() == 20, "b unaffected by a change");
}

void test_value_conversion_errors(TestStats& stats) { 
    {
        Value v;

        bool caught = false;
        try {
            v.AsInt();
        } catch (...) {
            caught = true;
        };
        check(stats, caught, "Calling asInt() on null value throws error");
        
        caught = false;
        try {
            v.AsString();
        } catch (...) {
            caught = true;
        }
        check(stats, caught, "Calling asString() on null value throws error");
    }

    {
        Value v(123);
        check(
            stats,
            v.AsInt() == 123,
            "Calling asInt() on int value returns int"
        );
        
        bool caught = false;
        try {
            v.AsString();
        } catch (...) {
            caught = true;
        }
        check(stats, caught, "Calling asString() on int value throws error");
    }

    {
        Value v("abc");
        
        bool caught = false;
        try {
            v.AsInt();
        } catch (...) {
            caught = true;
        }
        check(stats, caught, "Calling asInt() on string alue throws error");
        
        check(
            stats,
            v.AsString() == "abc",
            "Calling asString() on int value returns string"
        );
    }

}

int main() {
    TestStats stats;
    std::cout << "Running Value tests..." << std::endl;
    
    test_value_default_constructor(stats);
    test_value_int_constructor(stats);
    test_value_string_constructor(stats);
    test_value_null_value_properties(stats);
    test_value_type_consistency(stats);
    test_value_copy(stats);
    test_value_assignment(stats);
    test_value_boundary_values(stats);
    test_value_multiple_instances(stats);
    test_value_conversion_errors(stats);

    print_test_results(stats);
    if (stats.tests_failed > 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

