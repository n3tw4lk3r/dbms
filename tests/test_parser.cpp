#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

#include "test_utils.hpp"
#include "sqlparser/parser.hpp"
#include "sqlparser/command.hpp"

using namespace dbms;

void test_parse_empty(TestStats& stats) {
    test_header("Parse empty query");
    
    Parser parser;
    Command cmd = parser.parse("");
    check(stats, cmd.type == CommandType::kUnknown, "Empty query yields unknown command");
}

void test_parse_create_database(TestStats& stats) {
    test_header("Parse CREATE DATABASE");
    
    Parser parser;
    Command cmd = parser.parse("CREATE DATABASE mydb");
    
    check(stats, cmd.type == CommandType::kCreateDatabase, "Command type is kCreateDatabase");
    check(stats, cmd.database_name == "mydb", "Database name is mydb");
}

void test_parse_drop_database(TestStats& stats) {
    test_header("Parse DROP DATABASE");
    
    Parser parser;
    Command cmd = parser.parse("DROP DATABASE mydb");
    
    check(stats, cmd.type == CommandType::kDropDatabase, "Command type is kDropDatabase");
    check(stats, cmd.database_name == "mydb", "Database name is mydb");
}

void test_parse_use_database(TestStats& stats) {
    test_header("Parse USE DATABASE");
    
    Parser parser;
    Command cmd = parser.parse("USE mydb");
    
    check(stats, cmd.type == CommandType::kUseDatabase, "Command type is kUseDatabase");
    check(stats, cmd.database_name == "mydb", "Database name is mydb");
}

void test_parse_create_table(TestStats& stats) {
    test_header("Parse CREATE TABLE");
    
    Parser parser;
    Command cmd = parser.parse("CREATE TABLE mydb.users (id INT INDEXED, name STRING NOT_NULL)");
    
    check(stats, cmd.type == CommandType::kCreateTable, "Command type is kCreateTable");
    check(stats, cmd.database_name == "mydb", "Database name is mydb");
    check(stats, cmd.table_name == "users", "Table name is users");
    check(stats, cmd.columns.size() == 2, "Two columns");
    check(stats, cmd.columns[0].name == "id", "First column is id");
    check(stats, cmd.columns[0].type == ColumnType::kInt, "First column type is INT");
    check(stats, cmd.columns[0].indexed, "First column is indexed");
    check(stats, cmd.columns[0].not_null, "First column is not_null (from INDEXED)");
    check(stats, cmd.columns[1].name == "name", "Second column is name");
    check(stats, cmd.columns[1].type == ColumnType::kString, "Second column type is STRING");
    check(stats, cmd.columns[1].not_null, "Second column is not_null");
    check(stats, !cmd.columns[1].indexed, "Second column is not indexed");
}

void test_parse_create_table_without_db(TestStats& stats) {
    test_header("Parse CREATE TABLE without database");
    
    Parser parser;
    Command cmd = parser.parse("CREATE TABLE users (id INT)");
    
    check(stats, cmd.type == CommandType::kCreateTable, "Command type is kCreateTable");
    check(stats, cmd.database_name.empty(), "Database name is empty");
    check(stats, cmd.table_name == "users", "Table name is users");
    check(stats, cmd.columns.size() == 1, "One column");
}

void test_parse_drop_table(TestStats& stats) {
    test_header("Parse DROP TABLE");
    
    Parser parser;
    Command cmd = parser.parse("DROP TABLE mydb.users");
    
    check(stats, cmd.type == CommandType::kDropTable, "Command type is kDropTable");
    check(stats, cmd.database_name == "mydb", "Database name is mydb");
    check(stats, cmd.table_name == "users", "Table name is users");
}

void test_parse_insert(TestStats& stats) {
    test_header("Parse INSERT");
    
    Parser parser;
    Command cmd = parser.parse("INSERT INTO users (id, name) VALUE (1, \"john\")");
    
    check(stats, cmd.type == CommandType::kInsert, "Command type is kInsert");
    check(stats, cmd.table_name == "users", "Table name is users");
    check(stats, cmd.column_names.size() == 2, "Two column names");
    check(stats, cmd.column_names[0] == "id", "First column is id");
    check(stats, cmd.column_names[1] == "name", "Second column is name");
    check(stats, cmd.values.size() == 1, "One row of values");
    check(stats, cmd.values[0].size() == 2, "Two values in row");
    check(stats, cmd.values[0][0].getType() == Value::Type::kInt, "First value is int");
    check(stats, cmd.values[0][0].asInt() == 1, "First value is 1");
    check(stats, cmd.values[0][1].getType() == Value::Type::kString, "Second value is string");
    check(stats, cmd.values[0][1].asString() == "john", "Second value is john");
}

void test_parse_insert_multiple_rows(TestStats& stats) {
    test_header("Parse INSERT multiple rows");
    
    Parser parser;
    Command cmd = parser.parse("INSERT INTO users (a, b) VALUE (1, 2), (3, 4), (5, 6)");
    
    check(stats, cmd.type == CommandType::kInsert, "Command type is kInsert");
    check(stats, cmd.values.size() == 3, "Three rows");
    check(stats, cmd.values[0][0].asInt() == 1, "First row first value");
    check(stats, cmd.values[0][1].asInt() == 2, "First row second value");
    check(stats, cmd.values[1][0].asInt() == 3, "Second row first value");
    check(stats, cmd.values[1][1].asInt() == 4, "Second row second value");
    check(stats, cmd.values[2][0].asInt() == 5, "Third row first value");
    check(stats, cmd.values[2][1].asInt() == 6, "Third row second value");
}

void test_parse_insert_null(TestStats& stats) {
    test_header("Parse INSERT with NULL");
    
    Parser parser;
    Command cmd = parser.parse("INSERT INTO users (name) VALUE (NULL)");
    
    check(stats, cmd.values.size() == 1, "One row");
    check(stats, cmd.values[0].size() == 1, "One value");
    check(stats, cmd.values[0][0].isNull(), "Value is null");
}

void test_parse_select_all(TestStats& stats) {
    test_header("Parse SELECT *");
    
    Parser parser;
    Command cmd = parser.parse("SELECT * FROM mydb.users");
    
    check(stats, cmd.type == CommandType::kSelect, "Command type is kSelect");
    check(stats, cmd.database_name == "mydb", "Database name is mydb");
    check(stats, cmd.table_name == "users", "Table name is users");
    check(stats, cmd.select_columns.size() == 1, "One select column");
    check(stats, cmd.select_columns[0].name == "*", "Select column is *");
}

void test_parse_select_columns(TestStats& stats) {
    test_header("Parse SELECT specific columns");
    
    Parser parser;
    Command cmd = parser.parse("SELECT id, name FROM users");
    
    check(stats, cmd.type == CommandType::kSelect, "Command type is kSelect");
    check(stats, cmd.select_columns.size() == 2, "Two select columns");
    check(stats, cmd.select_columns[0].name == "id", "First column is id");
    check(stats, cmd.select_columns[1].name == "name", "Second column is name");
}

void test_parse_select_with_alias(TestStats& stats) {
    test_header("Parse SELECT with alias");
    
    Parser parser;
    Command cmd = parser.parse("SELECT id AS user_id, name AS user_name FROM users");
    
    check(stats, cmd.select_columns.size() == 2, "Two columns");
    check(stats, cmd.select_columns[0].name == "id", "First column name");
    check(stats, cmd.select_columns[0].alias == "user_id", "First column alias");
    check(stats, cmd.select_columns[1].name == "name", "Second column name");
    check(stats, cmd.select_columns[1].alias == "user_name", "Second column alias");
}

void test_parse_select_with_where(TestStats& stats) {
    test_header("Parse SELECT with WHERE");
    
    Parser parser;
    Command cmd = parser.parse("SELECT * FROM users WHERE id == 1");
    
    check(stats, cmd.conditions.size() == 1, "One condition");
    check(stats, cmd.conditions[0].lhs.is_column, "LHS is column");
    check(stats, cmd.conditions[0].lhs.column == "id", "LHS column is id");
    check(stats, cmd.conditions[0].operator_type == "==", "Operator is ==");
    check(stats, !cmd.conditions[0].rhs.is_column, "RHS is value");
    check(stats, cmd.conditions[0].rhs.value.asInt() == 1, "RHS value is 1");
}

void test_parse_select_with_multiple_conditions(TestStats& stats) {
    test_header("Parse SELECT with multiple conditions");
    
    Parser parser;
    Command cmd = parser.parse("SELECT * FROM users WHERE id >= 1 name LIKE \"^[A-Z]\"");
    
    check(stats, cmd.conditions.size() == 2, "Two conditions");
    check(stats, cmd.conditions[0].operator_type == ">=", "First operator is >=");
    check(stats, cmd.conditions[1].operator_type == "LIKE", "Second operator is LIKE");
}

void test_parse_select_with_between(TestStats& stats) {
    test_header("Parse SELECT with BETWEEN");
    
    Parser parser;
    Command cmd = parser.parse("SELECT * FROM users WHERE id BETWEEN 1 AND 10");
    
    check(stats, cmd.conditions.size() == 1, "One condition");
    check(stats, cmd.conditions[0].operator_type == "BETWEEN", "Operator is BETWEEN");
    check(stats, cmd.conditions[0].rhs.value.asInt() == 1, "Range start is 1");
    check(stats, cmd.conditions[0].range_end.value.asInt() == 10, "Range end is 10");
}

void test_parse_update(TestStats& stats) {
    test_header("Parse UPDATE");
    
    Parser parser;
    Command cmd = parser.parse("UPDATE users SET name = \"jane\" WHERE id == 1");
    
    check(stats, cmd.type == CommandType::kUpdate, "Command type is kUpdate");
    check(stats, cmd.table_name == "users", "Table name is users");
    check(stats, cmd.assignments.size() == 1, "One assignment");
    check(stats, cmd.assignments[0].column == "name", "Assignment column is name");
    check(stats, cmd.assignments[0].value.asString() == "jane", "Assignment value is jane");
    check(stats, cmd.conditions.size() == 1, "One condition");
}

void test_parse_update_multiple_assignments(TestStats& stats) {
    test_header("Parse UPDATE multiple assignments");
    
    Parser parser;
    Command cmd = parser.parse("UPDATE users SET name = \"jane\", age = 30 WHERE id == 1");
    
    check(stats, cmd.assignments.size() == 2, "Two assignments");
    check(stats, cmd.assignments[0].column == "name", "First assignment column");
    check(stats, cmd.assignments[0].value.asString() == "jane", "First assignment value");
    check(stats, cmd.assignments[1].column == "age", "Second assignment column");
    check(stats, cmd.assignments[1].value.asInt() == 30, "Second assignment value");
}

void test_parse_delete(TestStats& stats) {
    test_header("Parse DELETE");
    
    Parser parser;
    Command cmd = parser.parse("DELETE FROM users WHERE id == 1");
    
    check(stats, cmd.type == CommandType::kDelete, "Command type is kDelete");
    check(stats, cmd.table_name == "users", "Table name is users");
    check(stats, cmd.conditions.size() == 1, "One condition");
}

void test_parse_delete_no_where(TestStats& stats) {
    test_header("Parse DELETE without WHERE");
    
    Parser parser;
    Command cmd = parser.parse("DELETE FROM users");
    
    check(stats, cmd.type == CommandType::kDelete, "Command type is kDelete");
    check(stats, cmd.table_name == "users", "Table name is users");
    check(stats, cmd.conditions.empty(), "No conditions");
}

void test_parse_value_int(TestStats& stats) {
    test_header("Parse value int");
    
    Parser parser;
    Command cmd = parser.parse("INSERT INTO t (a) VALUE (42)");
    
    check(stats, cmd.values[0][0].getType() == Value::Type::kInt, "Type is int");
    check(stats, cmd.values[0][0].asInt() == 42, "Value is 42");
}

void test_parse_value_negative_int(TestStats& stats) {
    test_header("Parse value negative int");
    
    Parser parser;
    Command cmd = parser.parse("INSERT INTO t (a) VALUE (-10)");
    
    check(stats, cmd.values[0][0].getType() == Value::Type::kInt, "Type is int");
    check(stats, cmd.values[0][0].asInt() == -10, "Value is -10");
}

void test_parse_value_string(TestStats& stats) {
    test_header("Parse value string");
    
    Parser parser;
    Command cmd = parser.parse("INSERT INTO t (a) VALUE (\"hello world\")");
    
    check(stats, cmd.values[0][0].getType() == Value::Type::kString, "Type is string");
    check(stats, cmd.values[0][0].asString() == "hello world", "Value is hello world");
}

void test_parse_value_null(TestStats& stats) {
    test_header("Parse value NULL");
    
    Parser parser;
    Command cmd = parser.parse("INSERT INTO t (a) VALUE (NULL)");
    
    check(stats, cmd.values[0][0].isNull(), "Value is null");
}

void test_parse_condition_operators(TestStats& stats) {
    test_header("Parse condition operators");
    
    Parser parser;
    
    Command cmd1 = parser.parse("SELECT * FROM t WHERE a == 1");
    check(stats, cmd1.conditions[0].operator_type == "==", "Operator ==");
    
    Command cmd2 = parser.parse("SELECT * FROM t WHERE a != 1");
    check(stats, cmd2.conditions[0].operator_type == "!=", "Operator !=");
    
    Command cmd3 = parser.parse("SELECT * FROM t WHERE a < 1");
    check(stats, cmd3.conditions[0].operator_type == "<", "Operator <");
    
    Command cmd4 = parser.parse("SELECT * FROM t WHERE a > 1");
    check(stats, cmd4.conditions[0].operator_type == ">", "Operator >");
    
    Command cmd5 = parser.parse("SELECT * FROM t WHERE a <= 1");
    check(stats, cmd5.conditions[0].operator_type == "<=", "Operator <=");
    
    Command cmd6 = parser.parse("SELECT * FROM t WHERE a >= 1");
    check(stats, cmd6.conditions[0].operator_type == ">=", "Operator >=");
}

void test_parse_keywords_case_insensitive(TestStats& stats) {
    test_header("Parse case insensitive keywords");
    
    Parser parser;
    
    Command cmd1 = parser.parse("create database mydb");
    check(stats, cmd1.type == CommandType::kCreateDatabase, "Lowercase create works");
    
    Command cmd2 = parser.parse("CREATE database mydb");
    check(stats, cmd2.type == CommandType::kCreateDatabase, "Mixed case CREATE works");
    
    Command cmd3 = parser.parse("select * from users");
    check(stats, cmd3.type == CommandType::kSelect, "Lowercase select works");
    
    Command cmd4 = parser.parse("SELECT * FROM users");
    check(stats, cmd4.type == CommandType::kSelect, "Uppercase SELECT works");
}

void test_parse_invalid_identifier(TestStats& stats) {
    test_header("Parse invalid identifier");
    
    Parser parser;
    bool caught = false;
    try {
        parser.parse("CREATE DATABASE 123invalid");
    } catch (const std::runtime_error&) {
        caught = true;
    }
    check(stats, caught, "Invalid identifier starting with digit throws error");
}

void test_parse_table_with_db_prefix(TestStats& stats) {
    test_header("Parse table with database prefix");
    
    Parser parser;
    Command cmd = parser.parse("SELECT * FROM mydb.users");
    
    check(stats, cmd.database_name == "mydb", "Database name extracted");
    check(stats, cmd.table_name == "users", "Table name extracted");
}

void test_parse_insert_without_columns(TestStats& stats) {
    test_header("Parse INSERT without column names");
    
    Parser parser;
    Command cmd = parser.parse("INSERT INTO users VALUE (1, \"john\")");
    
    check(stats, cmd.type == CommandType::kInsert, "Command type is kInsert");
    check(stats, cmd.column_names.empty(), "No column names specified");
    check(stats, cmd.values.size() == 1, "One row of values");
    check(stats, cmd.values[0].size() == 2, "Two values in row");
}

int main() {
    TestStats stats;
    std::cout << "Running Parser tests..." << std::endl;
    
    test_parse_empty(stats);
    test_parse_create_database(stats);
    test_parse_drop_database(stats);
    test_parse_use_database(stats);
    test_parse_create_table(stats);
    test_parse_create_table_without_db(stats);
    test_parse_drop_table(stats);
    test_parse_insert(stats);
    test_parse_insert_multiple_rows(stats);
    test_parse_insert_null(stats);
    test_parse_select_all(stats);
    test_parse_select_columns(stats);
    test_parse_select_with_alias(stats);
    test_parse_select_with_where(stats);
    test_parse_select_with_multiple_conditions(stats);
    test_parse_select_with_between(stats);
    test_parse_update(stats);
    test_parse_update_multiple_assignments(stats);
    test_parse_delete(stats);
    test_parse_delete_no_where(stats);
    test_parse_value_int(stats);
    test_parse_value_negative_int(stats);
    test_parse_value_string(stats);
    test_parse_value_null(stats);
    test_parse_condition_operators(stats);
    test_parse_keywords_case_insensitive(stats);
    test_parse_invalid_identifier(stats);
    test_parse_table_with_db_prefix(stats);
    test_parse_insert_without_columns(stats);
    
    print_test_results(stats);
    return stats.tests_failed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

