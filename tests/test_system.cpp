#include <iostream>
#include <string>
#include <filesystem>
#include <stdexcept>

#include "test_utils.hpp"
#include "catalog/system.hpp"
#include "catalog/database.hpp"

using namespace dbms;
namespace fs = std::filesystem;

void test_system_initialization(TestStats& stats) {
    test_header("System initialization");
    
    fs::path test_root = "test_system_data";
    fs::remove_all(test_root);
    
    fs::create_directories(test_root);
    
    System system;
    
    check(stats, system.getCurrentDatabase() == nullptr, 
          "No current database on init");
    
    fs::remove_all(test_root);
}

void test_create_database(TestStats& stats) {
    test_header("Create database");
    
    fs::remove_all("test_system_data");
    
    System system;
    system.createDatabase("mydb");
    
    Database* db = system.getDatabase("mydb");
    check(stats, db != nullptr, "Database can be retrieved after creation");
    check(stats, db->getName() == "mydb", "Database name is correct");
    check(stats, fs::exists("test_system_data/mydb"), "Database directory exists");
    
    fs::remove_all("test_system_data");
}

void test_create_multiple_databases(TestStats& stats) {
    test_header("Create multiple databases");
    
    fs::remove_all("test_system_data");
    
    System system;
    system.createDatabase("db1");
    system.createDatabase("db2");
    system.createDatabase("db3");
    
    check(stats, system.getDatabase("db1") != nullptr, "db1 exists");
    check(stats, system.getDatabase("db2") != nullptr, "db2 exists");
    check(stats, system.getDatabase("db3") != nullptr, "db3 exists");
    check(stats, system.getDatabase("db1")->getName() == "db1", "db1 name correct");
    check(stats, system.getDatabase("db2")->getName() == "db2", "db2 name correct");
    check(stats, system.getDatabase("db3")->getName() == "db3", "db3 name correct");
    
    fs::remove_all("test_system_data");
}

void test_get_nonexistent_database(TestStats& stats) {
    test_header("Get nonexistent database");
    
    fs::remove_all("test_system_data");
    
    System system;
    
    check(stats, system.getDatabase("nonexistent") == nullptr, 
          "Nonexistent database returns nullptr");
    check(stats, system.getDatabase("") == nullptr, 
          "Empty name returns nullptr");
    
    fs::remove_all("test_system_data");
}

void test_use_database(TestStats& stats) {
    test_header("Use database");
    
    fs::remove_all("test_system_data");
    
    System system;
    system.createDatabase("mydb");
    system.useDatabase("mydb");
    
    Database* current = system.getCurrentDatabase();
    check(stats, current != nullptr, "Current database is not null after use");
    check(stats, current->getName() == "mydb", "Current database name is correct");
    
    fs::remove_all("test_system_data");
}

void test_use_nonexistent_database(TestStats& stats) {
    test_header("Use nonexistent database");
    
    fs::remove_all("test_system_data");
    
    System system;
    
    bool caught = false;
    try {
        system.useDatabase("nonexistent");
    } catch (const std::runtime_error&) {
        caught = true;
    }
    check(stats, caught, "Using nonexistent database throws runtime_error");
    check(stats, system.getCurrentDatabase() == nullptr, 
          "Current database is still null after failed use");
    
    fs::remove_all("test_system_data");
}

void test_switch_database(TestStats& stats) {
    test_header("Switch database");
    
    fs::remove_all("test_system_data");
    
    System system;
    system.createDatabase("first_db");
    system.createDatabase("second_db");
    
    system.useDatabase("first_db");
    check(stats, system.getCurrentDatabase()->getName() == "first_db", 
          "Current is first_db");
    
    system.useDatabase("second_db");
    check(stats, system.getCurrentDatabase()->getName() == "second_db", 
          "Current switched to second_db");
    
    fs::remove_all("test_system_data");
}

void test_drop_database(TestStats& stats) {
    test_header("Drop database");
    
    fs::remove_all("test_system_data");
    
    System system;
    system.createDatabase("mydb");
    check(stats, system.getDatabase("mydb") != nullptr, "Database exists before drop");
    
    system.dropDatabase("mydb");
    check(stats, system.getDatabase("mydb") == nullptr, "Database does not exist after drop");
    check(stats, !fs::exists("test_system_data/mydb"), "Database directory removed");
    
    fs::remove_all("test_system_data");
}

void test_drop_current_database(TestStats& stats) {
    test_header("Drop current database");
    
    fs::remove_all("test_system_data");
    
    System system;
    system.createDatabase("mydb");
    system.useDatabase("mydb");
    check(stats, system.getCurrentDatabase() != nullptr, "Current database set");
    
    system.dropDatabase("mydb");
    check(stats, system.getCurrentDatabase() == nullptr, 
          "Current database is null after dropping it");
    check(stats, system.getDatabase("mydb") == nullptr, 
          "Database removed");
    
    fs::remove_all("test_system_data");
}

void test_drop_nonexistent_database(TestStats& stats) {
    test_header("Drop nonexistent database");
    
    fs::remove_all("test_system_data");
    
    System system;
    system.dropDatabase("nonexistent");
    check(stats, true, "Dropping nonexistent database does not throw");
    
    fs::remove_all("test_system_data");
}

void test_create_database_with_tables(TestStats& stats) {
    test_header("Create database and add tables");
    
    fs::remove_all("test_system_data");
    
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
    
    fs::remove_all("test_system_data");
}

void test_get_current_database_when_none(TestStats& stats) {
    test_header("Get current database when none selected");
    
    fs::remove_all("test_system_data");
    
    System system;
    check(stats, system.getCurrentDatabase() == nullptr, 
          "Current database is null when none selected");
    
    system.createDatabase("testdb");
    check(stats, system.getCurrentDatabase() == nullptr, 
          "Current database still null after creating (not using)");
    
    fs::remove_all("test_system_data");
}

void test_system_persistence(TestStats& stats) {
    test_header("System persistence across instances");
    
    fs::remove_all("test_system_data");
    
    {
        System system;
        system.createDatabase("persist_db");
        Database* db = system.getDatabase("persist_db");
        std::vector<ColumnSchema> schema = {{"id", ColumnType::kInt}};
        db->createTable("items", schema);
    }
    
    {
        System system;
        Database* db = system.getDatabase("persist_db");
        check(stats, db != nullptr, "Database loaded from disk");
        check(stats, db->getName() == "persist_db", "Database name preserved");
        
        Table* table = db->getTable("items");
        check(stats, table != nullptr, "Table loaded from disk");
        check(stats, table->getName() == "items", "Table name preserved");
    }
    
    fs::remove_all("test_system_data");
}

int main() {
    TestStats stats;
    std::cout << "Running System tests..." << std::endl;
    
    test_system_initialization(stats);
    test_create_database(stats);
    test_create_multiple_databases(stats);
    test_get_nonexistent_database(stats);
    test_use_database(stats);
    test_use_nonexistent_database(stats);
    test_switch_database(stats);
    test_drop_database(stats);
    test_drop_current_database(stats);
    test_drop_nonexistent_database(stats);
    test_create_database_with_tables(stats);
    test_get_current_database_when_none(stats);
    test_system_persistence(stats);
    
    print_test_results(stats);
    return stats.tests_failed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

