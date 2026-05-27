#include <climits>
#include <iostream>
#include <string>

#include "test_utils.hpp"
#include "common/value.hpp"
#include "storage/row.hpp"
#include "storage/serializer.hpp"

using namespace dbms;

void test_serialize_null_value(TestStats& stats) {
    test_header("serialize null value");
    Value v;
    std::string result = Serializer::serializeValue(v);
    check(stats, result == "NULL", "null serializes to NULL");
}

void test_serialize_int_value(TestStats& stats) {
    test_header("serialize int value");
    check(
        stats,
        Serializer::serializeValue(Value(0)) == "INT:0",
        "0 -> INT:0"
    );
    check(
        stats,
        Serializer::serializeValue(Value(42)) == "INT:42",
        "42 -> INT:42"
    );
    check(
        stats,
        Serializer::serializeValue(Value(-100)) == "INT:-100",
        "-100 -> INT:-100"
    );
    check(
        stats,
        Serializer::serializeValue(
            Value(INT_MAX)
        ) == "INT:" + std::to_string(INT_MAX),
        "INT_MAX serialized"
    );
    check(
        stats,
        Serializer::serializeValue(
            Value(INT_MIN)
        ) == "INT:" + std::to_string(INT_MIN),
        "INT_MIN serialized"
    );
}

void test_serialize_string_value(TestStats& stats) {
    test_header("serialize string value");
    
    std::string result = Serializer::serializeValue(Value("hello"));
    check(stats, result == "STRING:5:hello", "hello serializes with length 5");
    
    result = Serializer::serializeValue(Value(""));
    check(stats, result == "STRING:0:", "empty string serializes with length 0");
    
    result = Serializer::serializeValue(Value("hello world"));
    check(
        stats,
        result == "STRING:11:hello world",
        "string with space serialized")
    ;
    
    std::string long_str(100, 'x');
    result = Serializer::serializeValue(Value(long_str));
    check(
        stats,
        result == "STRING:100:" + long_str,
        "long string serialized correctly"
    );
    
    result = Serializer::serializeValue(Value("12345"));
    check(
        stats,
        result == "STRING:5:12345",
        "numeric string serialized as string"
    );
}

void test_deserialize_null_value(TestStats& stats) {
    test_header("deserialize null value");
    Value v = Serializer::deserializeValue("NULL");
    check(stats, v.isNull(), "NULL deserializes to null");
    check(stats, v.getType() == Value::Type::kNull, "Type is kNull");
}

void test_deserialize_int_value(TestStats& stats) {
    test_header("deserialize int value");
    
    Value v = Serializer::deserializeValue("INT:42");
    check(stats, v.getType() == Value::Type::kInt, "Type is kInt");
    check(stats, v.asInt() == 42, "Value is 42");
    
    v = Serializer::deserializeValue("INT:0");
    check(stats, v.asInt() == 0, "0 deserialized");
    
    v = Serializer::deserializeValue("INT:-999");
    check(stats, v.asInt() == -999, "Negative deserialized");
    
    v = Serializer::deserializeValue("INT:2147483647");
    check(stats, v.asInt() == 2147483647, "Large int deserialized");
}

void test_deserialize_string_value(TestStats& stats) {
    test_header("deserialize string value");
    
    Value v = Serializer::deserializeValue("STRING:5:hello");
    check(stats, v.getType() == Value::Type::kString, "Type is kString");
    check(stats, v.asString() == "hello", "String value preserved");
    
    v = Serializer::deserializeValue("STRING:0:");
    check(
        stats,
        v.getType() == Value::Type::kString,
        "Empty string type is kString"
    );
    check(stats, v.asString().empty(), "Empty string deserialized");
    
    v = Serializer::deserializeValue("STRING:11:hello world");
    check(stats, v.asString() == "hello world", "String with space preserved");
    
    v = Serializer::deserializeValue("STRING:5:12345");
    check(stats, v.asString() == "12345", "Numeric string preserved");
}

void test_deserialize_corrupted_value(TestStats& stats) {
    test_header("deserialize corrupted value");
    
    bool caught = false;
    try {
        Serializer::deserializeValue("INVALID:xxx");
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "Invalid format throws error");
    
    caught = false;
    try {
        Serializer::deserializeValue("STRING:5:hello_world");
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "String size mismatch throws error");
    
    caught = false;
    try {
        Serializer::deserializeValue("STRING:abc:hello");
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "Corrupted string format throws error");
    
    caught = false;
    try {
        Serializer::deserializeValue("");
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "Empty string throws error");
    
    caught = false;
    try {
        Serializer::deserializeValue("INT:");
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "INT with no value throws error");
    
    caught = false;
    try {
        Serializer::deserializeValue("STRING:10:short");
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "String too short throws error");
}

void test_serialize_deserialize_roundtrip(TestStats& stats) {
    test_header("serialize/deserialize roundtrip");
    
    Value original_int(42);
    std::string serialized = Serializer::serializeValue(original_int);
    Value deserialized = Serializer::deserializeValue(serialized);
    check(
        stats,
        deserialized.getType() == original_int.getType(),
        "Int type roundtrip"
    );
    check(
        stats,
        deserialized.asInt() == original_int.asInt(),
        "Int value roundtrip"
    );
    
    Value original_str("hello world!");
    serialized = Serializer::serializeValue(original_str);
    deserialized = Serializer::deserializeValue(serialized);
    check(
        stats,
        deserialized.getType() == original_str.getType(),
        "String type roundtrip"
    );
    check(
        stats,
        deserialized.asString() == original_str.asString(),
        "String value roundtrip"
    );
    
    Value original_null;
    serialized = Serializer::serializeValue(original_null);
    deserialized = Serializer::deserializeValue(serialized);
    check(stats, deserialized.isNull(), "Null roundtrip");
    
    Value original_neg(-999);
    serialized = Serializer::serializeValue(original_neg);
    deserialized = Serializer::deserializeValue(serialized);
    check(stats, deserialized.asInt() == -999, "Negative int roundtrip");
}

void test_serialize_row_body(TestStats& stats) {
    test_header("serialize row body");
    
    Row row;
    row.values = {Value(1), Value("hello"), Value()};
    std::string body = Serializer::serializeRowBody(row);
    check(
        stats,
        body == "INT:1|STRING:5:hello|NULL",
        "Row body serialized correctly"
    );
    
    Row empty_row;
    check(
        stats,
        Serializer::serializeRowBody(empty_row).empty(),
        "Empty row body is empty"
    );
    
    Row single_value;
    single_value.values = {Value(42)};
    check(
        stats,
        Serializer::serializeRowBody(single_value) == "INT:42",
        "Single value row body"
    );
    
    Row two_values;
    two_values.values = {Value("a"), Value("b")};
    check(
        stats,
        Serializer::serializeRowBody(two_values) ==
        "STRING:1:a|STRING:1:b", "Two string values"
    );
}

void test_serialize_row(TestStats& stats) {
    test_header("serialize row");
    
    Row row;
    row.id = 42;
    row.values = {Value(10), Value("test")};
    std::string serialized = Serializer::serializeRow(row);
    check(stats, serialized == "42|INT:10|STRING:4:test", "Full row serialized");
    
    row.id = 0;
    row.values.clear();
    serialized = Serializer::serializeRow(row);
    check(stats, serialized == "0|", "Row with no values serialized");
    
    row.id = 999;
    row.values = {Value()};
    serialized = Serializer::serializeRow(row);
    check(stats, serialized == "999|NULL", "Row with null serialized");
}

void test_deserialize_row(TestStats& stats) {
    test_header("deserialize row");
    
    std::string line = "42|INT:10|STRING:4:test";
    Row row = Serializer::deserializeRow(line);
    check(stats, row.id == 42, "Row ID deserialized");
    check(stats, row.values.size() == 2, "Two values deserialized");
    check(
        stats,
        row.values[0].getType() == Value::Type::kInt,
        "First value type is int"
    );
    check(stats, row.values[0].asInt() == 10, "First value is 10");
    check(
        stats,
        row.values[1].getType() == Value::Type::kString,
        "Second value type is string"
    );
    check(stats, row.values[1].asString() == "test", "Second value is test");
    
    line = "1|NULL|INT:0|STRING:0:";
    row = Serializer::deserializeRow(line);
    check(stats, row.id == 1, "Row ID deserialized");
    check(stats, row.values.size() == 3, "Three values deserialized");
    check(stats, row.values[0].isNull(), "First value is null");
    check(stats, row.values[1].asInt() == 0, "Second value is 0");
    check(stats, row.values[2].asString().empty(), "Third value is empty string");
    
    line = "18446744073709551615|NULL";
    row = Serializer::deserializeRow(line);
    check(stats, row.id == 18446744073709551615ULL, "Max RowId deserialized");
    check(stats, row.values.size() == 1, "One value deserialized");
    check(stats, row.values[0].isNull(), "Value is null");
}

void test_serialize_deserialize_row_roundtrip(TestStats& stats) {
    test_header("row roundtrip");
    
    Row original;
    original.id = 12345;
    original.values = {Value(-50), Value("some text"), Value(999), Value()};
    
    std::string serialized = Serializer::serializeRow(original);
    Row deserialized = Serializer::deserializeRow(serialized);
    
    check(stats, deserialized.id == original.id, "Row ID roundtrip");
    check(
        stats,
        deserialized.values.size() == original.values.size(),
        "Values count roundtrip"
    );
    
    for (size_t i = 0; i < original.values.size(); ++i) {
        check(
            stats,
            deserialized.values[i].getType() == original.values[i].getType(), 
            "Value type roundtrip at index " + std::to_string(i)
        );
        if (original.values[i].getType() == Value::Type::kInt) {
            check(
                stats,
                deserialized.values[i].asInt() == original.values[i].asInt(),
                "Int value roundtrip at index " + std::to_string(i)
            );
        } else if (original.values[i].getType() == Value::Type::kString) {
            check(
                stats,
                deserialized.values[i].asString() == original.values[i].asString(),
                "String value roundtrip at index " + std::to_string(i)
            );
        }
    }
    
    Row empty_original;
    empty_original.id = 0;
    serialized = Serializer::serializeRow(empty_original);
    Row empty_deserialized = Serializer::deserializeRow(serialized);
    check(stats, empty_deserialized.id == 0, "Empty row ID roundtrip");
    check(stats, empty_deserialized.values.empty(), "Empty row values roundtrip");
}

int main() {
    TestStats stats;
    std::cout << "Running Serializer tests..." << std::endl;
    
    test_serialize_null_value(stats);
    test_serialize_int_value(stats);
    test_serialize_string_value(stats);
    test_deserialize_null_value(stats);
    test_deserialize_int_value(stats);
    test_deserialize_string_value(stats);
    test_deserialize_corrupted_value(stats);
    test_serialize_deserialize_roundtrip(stats);
    test_serialize_row_body(stats);
    test_serialize_row(stats);
    test_deserialize_row(stats);
    test_serialize_deserialize_row_roundtrip(stats);
    
    print_test_results(stats);
    if (stats.tests_failed > 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

