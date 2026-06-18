#include <iostream>
#include <string>

#include "catalog/system.hpp"
#include "catalog/database.hpp"
#include "utils.hpp"

using namespace dbms;

void test_system_initialization(TestStats& stats) {
    test_header("System initialization");

    ScopedDataDirectory guard;
    System system;

    check(
        stats,
        system.GetCurrentDatabase() == nullptr,
        "No current database on init"
    );
}

void test_system_create_database(TestStats& stats) {
    test_header("Create database");

    ScopedDataDirectory guard;
    System system;

    system.CreateDatabase("mydb");

    Database* db = system.GetDatabase("mydb");
    check(stats, db != nullptr, "Database can be retrieved after creation");
    check(stats, db->GetName() == "mydb", "Database name is correct");
    check(
        stats,
        std::filesystem::exists("data/mydb"),
        "Database directory exists"
    );
}

void test_system_create_multiple_databases(TestStats& stats) {
    test_header("Create multiple databases");

    ScopedDataDirectory guard;
    System system;

    system.CreateDatabase("db1");
    system.CreateDatabase("db2");
    system.CreateDatabase("db3");

    check(stats, system.GetDatabase("db1") != nullptr, "db1 exists");
    check(stats, system.GetDatabase("db2") != nullptr, "db2 exists");
    check(stats, system.GetDatabase("db3") != nullptr, "db3 exists");
    check(
        stats,
        system.GetDatabase("db1")->GetName() == "db1",
        "db1 name correct"
    );
    check(
        stats,
        system.GetDatabase("db2")->GetName() == "db2",
        "db2 name correct"
    );
    check(
        stats,
        system.GetDatabase("db3")->GetName() == "db3",
        "db3 name correct"
    );
}

void test_system_get_nonexistent_database(TestStats& stats) {
    test_header("Get nonexistent database");

    ScopedDataDirectory guard;
    System system;

    check(
        stats,
        system.GetDatabase("nonexistent") == nullptr,
        "Nonexistent database returns nullptr"
    );
    check(
        stats,
        system.GetDatabase("") == nullptr,
        "Empty name returns nullptr"
    );
}

void test_system_use_database(TestStats& stats) {
    test_header("Use database");

    ScopedDataDirectory guard;
    System system;

    system.CreateDatabase("mydb");
    system.UseDatabase("mydb");

    Database* current = system.GetCurrentDatabase();
    check(stats, current != nullptr, "Current database is not null after use");
    check(
        stats,
        current->GetName() == "mydb",
        "Current database name is correct"
    );
}

void test_system_use_nonexistent_database(TestStats& stats) {
    test_header("Use nonexistent database");

    ScopedDataDirectory guard;

    System system;

    bool caught = false;
    try {
        system.UseDatabase("nonexistent");
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "Using nonexistent database throws error");
    check(
        stats,
        system.GetCurrentDatabase() == nullptr,
        "Current database is still null after failed use"
    );
}

void test_system_switch_database(TestStats& stats) {
    test_header("Switch database");

    ScopedDataDirectory guard;
    System system;

    system.CreateDatabase("first_db");
    system.CreateDatabase("second_db");

    system.UseDatabase("first_db");
    check(
        stats,
        system.GetCurrentDatabase()->GetName() == "first_db",
        "Current is first_db"
    );

    system.UseDatabase("second_db");
    check(
        stats,
        system.GetCurrentDatabase()->GetName() == "second_db",
        "Current switched to second_db"
    );
}

void test_system_drop_database(TestStats& stats) {
    test_header("Drop database");

    ScopedDataDirectory guard;
    System system;

    system.CreateDatabase("mydb");
    check(
        stats,
        system.GetDatabase("mydb") != nullptr,
        "Database exists before drop"
    );

    system.DropDatabase("mydb");
    check(
        stats,
        system.GetDatabase("mydb") == nullptr,
        "Database does not exist after drop"
    );
    check(
        stats,
        !std::filesystem::exists("data/mydb"),
        "Database directory removed"
    );
}

void test_system_drop_current_database(TestStats& stats) {
    test_header("Drop current database");

    ScopedDataDirectory guard;
    System system;

    system.CreateDatabase("mydb");
    system.UseDatabase("mydb");
    check(stats, system.GetCurrentDatabase() != nullptr, "Current database set");

    system.DropDatabase("mydb");
    check(
        stats,
        system.GetCurrentDatabase() == nullptr,
        "Current database is null after dropping it"
    );
    check(
        stats,
        system.GetDatabase("mydb") == nullptr,
        "Database removed"
    );
}

void test_system_drop_nonexistent_database(TestStats& stats) {
    test_header("Drop nonexistent database");

    ScopedDataDirectory guard;
    System system;

    system.DropDatabase("nonexistent");
    check(stats, true, "Dropping nonexistent database does not throw");
}

void test_system_create_database_with_tables(TestStats& stats) {
    test_header("Create database and add tables");

    ScopedDataDirectory guard;
    System system;

    system.CreateDatabase("appdb");
    system.UseDatabase("appdb");

    Database* db = system.GetCurrentDatabase();
    check(stats, db != nullptr, "Current database available");

    std::vector<ColumnSchema> schema = {{"id", ColumnType::kInt, true, true}};
    db->CreateTable("users", schema);
    db->CreateTable("products", schema);

    check(stats, db->GetTable("users") != nullptr, "Users table exists");
    check(stats, db->GetTable("products") != nullptr, "Products table exists");
}

void test_system_get_current_database_when_none(TestStats& stats) {
    test_header("Get current database when none selected");

    ScopedDataDirectory guard;
    System system;

    check(
        stats,
        system.GetCurrentDatabase() == nullptr,
        "Current database is null when none selected"
    );

    system.CreateDatabase("testdb");
    check(
        stats,
        system.GetCurrentDatabase() == nullptr,
        "Current database still null after creating (not using)"
    );
}

int main() {
    TestStats stats;
    std::cout << "Running System tests..." << std::endl;

    test_system_initialization(stats);
    test_system_create_database(stats);
    test_system_create_multiple_databases(stats);
    test_system_get_nonexistent_database(stats);
    test_system_use_database(stats);
    test_system_use_nonexistent_database(stats);
    test_system_switch_database(stats);
    test_system_drop_database(stats);
    test_system_drop_current_database(stats);
    test_system_drop_nonexistent_database(stats);
    test_system_create_database_with_tables(stats);
    test_system_get_current_database_when_none(stats);

    print_test_results(stats);
    if (stats.tests_failed > 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

