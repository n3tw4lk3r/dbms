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
        system.getCurrentDatabase() == nullptr, 
        "No current database on init"
    );
}

void test_system_create_database(TestStats& stats) {
    test_header("Create database");
    
    ScopedDataDirectory guard;
    System system;
    
    system.createDatabase("mydb");
    
    Database* db = system.getDatabase("mydb");
    check(stats, db != nullptr, "Database can be retrieved after creation");
    check(stats, db->getName() == "mydb", "Database name is correct");
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

    system.createDatabase("db1");
    system.createDatabase("db2");
    system.createDatabase("db3");
    
    check(stats, system.getDatabase("db1") != nullptr, "db1 exists");
    check(stats, system.getDatabase("db2") != nullptr, "db2 exists");
    check(stats, system.getDatabase("db3") != nullptr, "db3 exists");
    check(
        stats,
        system.getDatabase("db1")->getName() == "db1",
        "db1 name correct"
    );
    check(
        stats,
        system.getDatabase("db2")->getName() == "db2",
        "db2 name correct"
    );
    check(
        stats,
        system.getDatabase("db3")->getName() == "db3",
        "db3 name correct"
    );
}

void test_system_get_nonexistent_database(TestStats& stats) {
    test_header("Get nonexistent database");
    
    ScopedDataDirectory guard;
    System system;
    
    check(
        stats,
        system.getDatabase("nonexistent") == nullptr, 
        "Nonexistent database returns nullptr"
    );
    check(
        stats,
        system.getDatabase("") == nullptr, 
        "Empty name returns nullptr"
    );
}

void test_system_use_database(TestStats& stats) {
    test_header("Use database");
    
    ScopedDataDirectory guard;
    System system;

    system.createDatabase("mydb");
    system.useDatabase("mydb");
    
    Database* current = system.getCurrentDatabase();
    check(stats, current != nullptr, "Current database is not null after use");
    check(
        stats,
        current->getName() == "mydb",
        "Current database name is correct"
    );
}

void test_system_use_nonexistent_database(TestStats& stats) {
    test_header("Use nonexistent database");
    
    ScopedDataDirectory guard;
    
    System system;
    
    bool caught = false;
    try {
        system.useDatabase("nonexistent");
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "Using nonexistent database throws error");
    check(
        stats,
        system.getCurrentDatabase() == nullptr, 
        "Current database is still null after failed use"
    );
}

void test_system_switch_database(TestStats& stats) {
    test_header("Switch database");
    
    ScopedDataDirectory guard;
    System system;

    system.createDatabase("first_db");
    system.createDatabase("second_db");
    
    system.useDatabase("first_db");
    check(
        stats,
        system.getCurrentDatabase()->getName() == "first_db", 
        "Current is first_db"
    );
    
    system.useDatabase("second_db");
    check(
        stats,
        system.getCurrentDatabase()->getName() == "second_db", 
        "Current switched to second_db"
    );
}

void test_system_drop_database(TestStats& stats) {
    test_header("Drop database");
    
    ScopedDataDirectory guard;
    System system;

    system.createDatabase("mydb");
    check(
        stats,
        system.getDatabase("mydb") != nullptr,
        "Database exists before drop"
    );
    
    system.dropDatabase("mydb");
    check(
        stats,
        system.getDatabase("mydb") == nullptr,
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

    system.createDatabase("mydb");
    system.useDatabase("mydb");
    check(stats, system.getCurrentDatabase() != nullptr, "Current database set");
    
    system.dropDatabase("mydb");
    check(
        stats,
        system.getCurrentDatabase() == nullptr, 
        "Current database is null after dropping it"
    );
    check(
        stats,
        system.getDatabase("mydb") == nullptr, 
        "Database removed"
    );
}

void test_system_drop_nonexistent_database(TestStats& stats) {
    test_header("Drop nonexistent database");
    
    ScopedDataDirectory guard;
    System system;

    system.dropDatabase("nonexistent");
    check(stats, true, "Dropping nonexistent database does not throw");
}

void test_system_create_database_with_tables(TestStats& stats) {
    test_header("Create database and add tables");
    
    ScopedDataDirectory guard;
    System system;

    system.createDatabase("appdb");
    system.useDatabase("appdb");
    
    Database* db = system.getCurrentDatabase();
    check(stats, db != nullptr, "Current database available");
    
    std::vector<ColumnSchema> schema = {{"id", ColumnType::kInt, true, true}};
    db->createTable("users", schema);
    db->createTable("products", schema);
    
    check(stats, db->getTable("users") != nullptr, "Users table exists");
    check(stats, db->getTable("products") != nullptr, "Products table exists");
}

void test_system_get_current_database_when_none(TestStats& stats) {
    test_header("Get current database when none selected");
    
    ScopedDataDirectory guard;
    System system;

    check(
        stats,
        system.getCurrentDatabase() == nullptr, 
        "Current database is null when none selected"
    );
    
    system.createDatabase("testdb");
    check(
        stats,
        system.getCurrentDatabase() == nullptr, 
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

