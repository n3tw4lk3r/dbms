#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "catalog/Database.hpp"
#include "common/types.hpp"
#include "utils.hpp"

using namespace dbms;
namespace fs = std::filesystem;

void test_database_creation(TestStats& stats) {
    test_header("Database creation");

    fs::path test_path = "test_data/test_db_creation";
    fs::remove_all(test_path);

    Database db("test_db", test_path);

    check(stats, db.GetName() == "test_db", "Database name is correct");
    check(stats, fs::exists(test_path), "Storage directory created");
    check(stats, fs::is_directory(test_path), "Storage path is directory");

    fs::remove_all(test_path);
}

void test_database_get_name(TestStats& stats) {
    test_header("Get database name");

    fs::path test_path = "test_data/test_db_name";
    fs::remove_all(test_path);

    Database db1("first_db", test_path / "first");
    check(stats, db1.GetName() == "first_db", "First database name correct");

    Database db2("second_db", test_path / "second");
    check(stats, db2.GetName() == "second_db", "Second database name correct");

    Database db3("", test_path / "empty");
    check(stats, db3.GetName().empty(), "Empty database name");

    fs::remove_all(test_path);
}

void test_database_create_table(TestStats& stats) {
    test_header("Create table");

    fs::path test_path = "test_data/test_db_create_table";
    fs::remove_all(test_path);

    Database db("test_db", test_path);

    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true},
        {"name", ColumnType::kString, false, false}
    };

    db.CreateTable("users", schema);

    Table* table = db.GetTable("users");
    check(stats, table != nullptr, "Table can be retrieved after creation");
    check(stats, table->GetName() == "users", "Table name is correct");

    const auto& table_schema = table->GetSchema();
    check(stats, table_schema.size() == 2, "Table schema has 2 columns");
    check(stats, table_schema[0].name == "id", "First column name correct");
    check(
        stats,
        table_schema[0].type == ColumnType::kInt,
        "First column type correct"
    );
    check(stats, table_schema[0].not_null, "First column not_null correct");
    check(stats, table_schema[0].indexed, "First column indexed correct");
    check(stats, table_schema[1].name == "name", "Second column name correct");
    check(
        stats,
        table_schema[1].type == ColumnType::kString,
        "Second column type correct"
    );
    check(stats, !table_schema[1].not_null, "Second column not_null correct");
    check(stats, !table_schema[1].indexed, "Second column indexed correct");

    fs::remove_all(test_path);
}

void test_database_create_duplicate_table(TestStats& stats) {
    test_header("Create duplicate table");

    fs::path test_path = "test_data/test_db_dup_table";
    fs::remove_all(test_path);

    Database db("test_db", test_path);

    std::vector<ColumnSchema> schema = {{"id", ColumnType::kInt}};
    db.CreateTable("users", schema);

    bool caught = false;
    try {
        db.CreateTable("users", schema);
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "Creating duplicate table throws error");

    fs::remove_all(test_path);
}

void test_database_create_multiple_tables(TestStats& stats) {
    test_header("Create multiple tables");

    fs::path test_path = "test_data/test_db_multi_tables";
    fs::remove_all(test_path);

    Database db("test_db", test_path);

    std::vector<ColumnSchema> schema = {{"id", ColumnType::kInt}};
    db.CreateTable("table1", schema);
    db.CreateTable("table2", schema);
    db.CreateTable("table3", schema);

    check(stats, db.GetTable("table1") != nullptr, "Table1 exists");
    check(stats, db.GetTable("table2") != nullptr, "Table2 exists");
    check(stats, db.GetTable("table3") != nullptr, "Table3 exists");
    check(
        stats,
        db.GetTable("nonexistent") == nullptr,
        "Nonexistent table returns nullptr"
    );

    fs::remove_all(test_path);
}

void test_database_get_table_nonexistent(TestStats& stats) {
    test_header("Get nonexistent table");

    fs::path test_path = "test_data/test_db_nonexist";
    fs::remove_all(test_path);

    Database db("test_db", test_path);

    check(
        stats,
        db.GetTable("nonexistent") == nullptr,
        "Nonexistent table returns nullptr"
    );
    check(stats, db.GetTable("") == nullptr, "Empty name returns nullptr");

    fs::remove_all(test_path);
}

void test_database_drop_table(TestStats& stats) {
    test_header("Drop table");

    fs::path test_path = "test_data/test_db_drop";
    fs::remove_all(test_path);

    Database db("test_db", test_path);

    std::vector<ColumnSchema> schema = {{"id", ColumnType::kInt}};
    db.CreateTable("users", schema);
    check(stats, db.GetTable("users") != nullptr, "Table exists before drop");

    db.DropTable("users");
    check(
        stats,
        db.GetTable("users") == nullptr,
        "Table does not exist after drop"
    );

    fs::remove_all(test_path);
}

void test_database_drop_nonexistent_table(TestStats& stats) {
    test_header("Drop nonexistent table");

    fs::path test_path = "test_data/test_db_drop_nonexist";
    fs::remove_all(test_path);

    Database db("test_db", test_path);

    db.DropTable("nonexistent");
    check(stats, true, "Dropping nonexistent table does not throw");

    fs::remove_all(test_path);
}

void test_database_create_and_drop_repeatedly(TestStats& stats) {
    test_header("Create and drop repeatedly");

    fs::path test_path = "test_data/test_db_repeated";
    fs::remove_all(test_path);

    Database db("test_db", test_path);
    std::vector<ColumnSchema> schema = {{"id", ColumnType::kInt}};

    for (int i = 0; i < 5; ++i) {
        db.CreateTable("cycle_table", schema);
        check(
            stats,
            db.GetTable("cycle_table") != nullptr,
            "Table exists after create iteration " + std::to_string(i)
        );
        db.DropTable("cycle_table");
        check(
            stats,
            db.GetTable("cycle_table") == nullptr,
            "Table removed after drop iteration " + std::to_string(i)
        );
    }

    fs::remove_all(test_path);
}

void test_database_table_with_complex_schema(TestStats& stats) {
    test_header("Table with complex schema");

    fs::path test_path = "test_data/test_db_complex";
    fs::remove_all(test_path);

    Database db("test_db", test_path);

    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true},
        {"name", ColumnType::kString, true, false},
        {"age", ColumnType::kInt, false, false},
        {"email", ColumnType::kString, false, true},
        {"score", ColumnType::kInt, false, false}
    };

    db.CreateTable("complex_table", schema);

    Table* table = db.GetTable("complex_table");
    check(stats, table != nullptr, "Complex table created");

    const auto& table_schema = table->GetSchema();
    check(stats, table_schema.size() == 5, "Complex table has 5 columns");
    check(stats, table_schema[0].not_null, "id is not_null");
    check(stats, table_schema[0].indexed, "id is indexed");
    check(stats, table_schema[1].not_null, "name is not_null");
    check(stats, !table_schema[1].indexed, "name is not indexed");
    check(stats, !table_schema[2].not_null, "age is nullable");
    check(stats, !table_schema[2].indexed, "age is not indexed");
    check(stats, !table_schema[3].not_null, "email is nullable");
    check(stats, table_schema[3].indexed, "email is indexed");
    check(stats, !table_schema[4].not_null, "score is nullable");
    check(stats, !table_schema[4].indexed, "score is not indexed");

    fs::remove_all(test_path);
}

int main() {
    TestStats stats;
    std::cout << "Running Database tests..." << std::endl;

    test_database_creation(stats);
    test_database_get_name(stats);
    test_database_create_table(stats);
    test_database_create_duplicate_table(stats);
    test_database_create_multiple_tables(stats);
    test_database_get_table_nonexistent(stats);
    test_database_drop_table(stats);
    test_database_drop_nonexistent_table(stats);
    test_database_create_and_drop_repeatedly(stats);
    test_database_table_with_complex_schema(stats);

    print_test_results(stats);
    if (stats.tests_failed > 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

