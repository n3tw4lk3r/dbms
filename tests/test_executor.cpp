#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "catalog/system.hpp"
#include "execution/executor.hpp"
#include "sqlparser/parser.hpp"
#include "sqlparser/command.hpp"
#include "utils.hpp"

using namespace dbms;

struct TestExecutorContext {
    ScopedDataDirectory guard;
    System system;
    Parser parser;
    Executor executor;
    std::ostringstream output;
    std::streambuf* old_cout;
    std::ostringstream error_output;
    std::streambuf* old_cerr;
    
    TestExecutorContext() :
        executor(system)
    {
        old_cout = std::cout.rdbuf(output.rdbuf());
        old_cerr = std::cerr.rdbuf(error_output.rdbuf());
    }
    
    ~TestExecutorContext() {
        std::cout.rdbuf(old_cout);
        std::cerr.rdbuf(old_cerr);
    }
    
    void execute(const std::string& query) {
        Command cmd = parser.parse(query);
        executor.execute(cmd);
    }
    
    std::string getOutput() {
        return output.str();
    }
    
    void clearOutput() {
        output.str("");
        output.clear();
    }
};

void test_executor_execute_create_database(TestStats& stats) {
    test_header("Execute CREATE DATABASE");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    
    Database* db = ctx.system.getDatabase("testdb");
    check(stats, db != nullptr, "Database created");
    check(stats, db->getName() == "testdb", "Database name correct");
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("Database testdb created") != std::string::npos, 
        "Creation message contains database name"
    );
}

void test_executor_execute_drop_database(TestStats& stats) {
    test_header("Execute DROP DATABASE");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.clearOutput();
    
    ctx.execute("DROP DATABASE testdb");
    check(
        stats,
        ctx.system.getDatabase("testdb") == nullptr,
        "Database dropped"
    );
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("Database dropped: testdb") != std::string::npos, 
        "Drop message contains database name"
    );
}

void test_executor_execute_drop_nonexistent_database(TestStats& stats) {
    test_header("Execute DROP DATABASE nonexistent");
    
    TestExecutorContext ctx;
    
    bool caught = false;
    try {
        ctx.execute("DROP DATABASE nonexistent");
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "Dropping nonexistent database throws error");
}

void test_executor_execute_use_database(TestStats& stats) {
    test_header("Execute USE DATABASE");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.clearOutput();
    
    ctx.execute("USE testdb");
    check(
        stats,
        ctx.system.getCurrentDatabase() != nullptr,
        "Current database set"
    );
    check(
        stats,
        ctx.system.getCurrentDatabase()->getName() == "testdb", 
        "Current database name correct"
    );
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("Using database testdb") != std::string::npos, 
        "Use message contains database name"
    );
}

void test_executor_execute_create_table(TestStats& stats) {
    test_header("Execute CREATE TABLE");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.clearOutput();
    
    ctx.execute(
        "CREATE TABLE testdb.users (id INT INDEXED, name STRING NOT_NULL)"
    );
    
    Database* db = ctx.system.getCurrentDatabase();
    Table* table = db->getTable("users");
    check(stats, table != nullptr, "Table created");
    check(stats, table->getName() == "users", "Table name correct");
    check(stats, table->getSchema().size() == 2, "Schema has 2 columns");
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("Table users created") != std::string::npos, 
        "Creation message contains table name"
    );
}

void test_executor_execute_create_table_no_database(TestStats& stats) {
    test_header("Execute CREATE TABLE without database");
    
    TestExecutorContext ctx;
    
    bool caught = false;
    try {
        ctx.execute("CREATE TABLE users (id INT)");
    } catch (...) {
        caught = true;
    }
    check(
        stats,
        caught,
        "Creating table without database throws error"
    );
}

void test_executor_execute_drop_table(TestStats& stats) {
    test_header("Execute DROP TABLE");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.execute("CREATE TABLE testdb.users (id INT)");
    ctx.clearOutput();
    
    ctx.execute("DROP TABLE testdb.users");
    
    Database* db = ctx.system.getCurrentDatabase();
    check(stats, db->getTable("users") == nullptr, "Table dropped");
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("Table dropped: users") != std::string::npos, 
        "Drop message contains table name"
    );
}

void test_executor_execute_insert(TestStats& stats) {
    test_header("Execute INSERT");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.execute("CREATE TABLE testdb.users (id INT INDEXED, name STRING)");
    ctx.clearOutput();
    
    ctx.execute("INSERT INTO users (id, name) VALUE (1, \"alice\")");
    
    Database* db = ctx.system.getCurrentDatabase();
    Table* table = db->getTable("users");
    const auto& rows = table->getRows();
    check(stats, rows.size() == 1, "One row inserted");
    check(stats, rows[0]->values[0].asInt() == 1, "ID value correct");
    check(stats, rows[0]->values[1].asString() == "alice", "Name value correct");
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("1 rows inserted") != std::string::npos, 
        "Insert message shows row count"
    );
}

void test_executor_execute_insert_multiple_rows(TestStats& stats) {
    test_header("Execute INSERT multiple rows");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.execute("CREATE TABLE testdb.users (id INT INDEXED, name STRING)");
    ctx.clearOutput();
   
    std::string data = "INSERT INTO users (id, name) VALUE (1, \"alice\"),";
    data += "(2, \"bob\"), (3, \"charlie\")";
    ctx.execute(data);
    
    Database* db = ctx.system.getCurrentDatabase();
    Table* table = db->getTable("users");
    check(stats, table->getRows().size() == 3, "Three rows inserted");
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("3 rows inserted") != std::string::npos, 
        "Insert message shows correct count"
    );
}

void test_executor_execute_insert_no_table(TestStats& stats) {
    test_header("Execute INSERT no table");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    
    bool caught = false;
    try {
        ctx.execute("INSERT INTO nonexistent (id) VALUE (1)");
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "Insert into nonexistent table throws error");
}

void test_executor_execute_select(TestStats& stats) {
    test_header("Execute SELECT");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.execute(
        "CREATE TABLE testdb.users (id INT INDEXED, name STRING, age INT)"
    );
    ctx.execute("INSERT INTO users (id, name, age) VALUE (1, \"alice\", 25)");
    ctx.execute("INSERT INTO users (id, name, age) VALUE (2, \"bob\", 30)");
    ctx.clearOutput();
    
    ctx.execute("SELECT id, name, age FROM users");
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("\"id\": 1") != std::string::npos,
        "Output contains id 1"
    );
    check(
        stats,
        out.find("\"name\": \"alice\"") != std::string::npos,
        "Output contains alice"
    );
    check(
        stats,
        out.find("\"age\": 25") != std::string::npos,
        "Output contains age 25"
    );
    check(
        stats,
        out.find("\"id\": 2") != std::string::npos,
        "Output contains id 2"
    );
    check(
        stats,
        out.find("\"name\": \"bob\"") != std::string::npos,
        "Output contains bob"
    );
    check(
        stats,
        out.find("\"age\": 30") != std::string::npos,
        "Output contains age 30"
    );
    check(stats, out.find("[") != std::string::npos, "Output is JSON array");
    check(stats, out.find("]") != std::string::npos, "Output is JSON array");
}

void test_executor_execute_select_all(TestStats& stats) {
    test_header("Execute SELECT *");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.execute("CREATE TABLE testdb.users (id INT INDEXED, name STRING)");
    ctx.execute("INSERT INTO users (id, name) VALUE (1, \"alice\")");
    ctx.clearOutput();
    
    ctx.execute("SELECT * FROM users");
    
    std::string out = ctx.getOutput();
    check(stats, out.find("\"id\": 1") != std::string::npos, "Contains id");
    check(
        stats,
        out.find("\"name\": \"alice\"") != std::string::npos,
        "Contains name"
    );
}

void test_executor_execute_select_with_where(TestStats& stats) {
    test_header("Execute SELECT with WHERE");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.execute("CREATE TABLE testdb.users (id INT INDEXED, name STRING)");
    ctx.execute("INSERT INTO users (id, name) VALUE (1, \"alice\")");
    ctx.execute("INSERT INTO users (id, name) VALUE (2, \"bob\")");
    ctx.clearOutput();
    
    ctx.execute("SELECT * FROM users WHERE id == 1");
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("\"id\": 1") != std::string::npos,
        "Contains matching row"
    );
    check(
        stats,
        out.find("\"id\": 2") == std::string::npos,
        "Does not contain non-matching row"
    );
}

void test_executor_execute_select_with_alias(TestStats& stats) {
    test_header("Execute SELECT with alias");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.execute("CREATE TABLE testdb.users (id INT, name STRING)");
    ctx.execute("INSERT INTO users (id, name) VALUE (1, \"alice\")");
    ctx.clearOutput();
    
    ctx.execute("SELECT id AS user_id, name AS user_name FROM users");
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("\"user_id\": 1") != std::string::npos,
        "Alias user_id appears"
    );
    check(
        stats,
        out.find("\"user_name\": \"alice\"") != std::string::npos,
        "Alias user_name appears"
    );
    check(
        stats,
        out.find("\"id\":") == std::string::npos,
        "Original column name not in output"
    );
}

void test_executor_execute_select_no_table(TestStats& stats) {
    test_header("Execute SELECT no table");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    
    bool caught = false;
    try {
        ctx.execute("SELECT * FROM nonexistent");
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "Select from nonexistent table throws error");
}

void test_executor_execute_update(TestStats& stats) {
    test_header("Execute UPDATE");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.execute(
        "CREATE TABLE testdb.users (id INT INDEXED, name STRING, age INT)"
    );
    ctx.execute("INSERT INTO users (id, name, age) VALUE (1, \"alice\", 25)");
    ctx.execute("INSERT INTO users (id, name, age) VALUE (2, \"bob\", 30)");
    ctx.clearOutput();
    
    ctx.execute("UPDATE users SET name = \"alice_new\", age = 26 WHERE id == 1");
    
    Database* db = ctx.system.getCurrentDatabase();
    Table* table = db->getTable("users");
    Row* row = table->findRowById(1);
    check(stats, row->values[1].asString() == "alice_new", "Name updated");
    check(stats, row->values[2].asInt() == 26, "Age updated");
    
    row = table->findRowById(2);
    check(stats, row->values[1].asString() == "bob", "Other row unchanged");
    check(stats, row->values[2].asInt() == 30, "Other row age unchanged");
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("1 rows updated") != std::string::npos,
        "Update message correct"
    );
}

void test_executor_execute_update_multiple_rows(TestStats& stats) {
    test_header("Execute UPDATE multiple rows");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.execute("CREATE TABLE testdb.users (id INT, name STRING)");
    ctx.execute("INSERT INTO users (id, name) VALUE (1, \"alice\")");
    ctx.execute("INSERT INTO users (id, name) VALUE (2, \"bob\")");
    ctx.execute("INSERT INTO users (id, name) VALUE (3, \"charlie\")");
    ctx.clearOutput();
    
    ctx.execute("UPDATE users SET name = \"updated\" WHERE id >= 2");
    
    Database* db = ctx.system.getCurrentDatabase();
    Table* table = db->getTable("users");
    check(
        stats,
        table->findRowById(1)->values[1].asString() == "alice",
        "Row 1 unchanged"
    );
    check(
        stats,
        table->findRowById(2)->values[1].asString() == "updated",
        "Row 2 updated"
    );
    check(
        stats,
        table->findRowById(3)->values[1].asString() == "updated",
        "Row 3 updated"
    );
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("2 rows updated") != std::string::npos,
        "Two rows updated"
    );
}

void test_executor_execute_delete(TestStats& stats) {
    test_header("Execute DELETE");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.execute("CREATE TABLE testdb.users (id INT INDEXED, name STRING)");
    ctx.execute("INSERT INTO users (id, name) VALUE (1, \"alice\")");
    ctx.execute("INSERT INTO users (id, name) VALUE (2, \"bob\")");
    ctx.execute("INSERT INTO users (id, name) VALUE (3, \"charlie\")");
    ctx.clearOutput();
    
    ctx.execute("DELETE FROM users WHERE id == 2");
    
    Database* db = ctx.system.getCurrentDatabase();
    Table* table = db->getTable("users");
    
    check(stats, !table->findRowById(1)->deleted, "Row 1 not deleted");
    check(stats, table->findRowById(2)->deleted, "Row 2 deleted");
    check(stats, !table->findRowById(3)->deleted, "Row 3 not deleted");
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("1 rows deleted") != std::string::npos,
        "Delete message correct"
    );
}

void test_executor_execute_delete_all(TestStats& stats) {
    test_header("Execute DELETE all");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.execute("CREATE TABLE testdb.users (id INT, name STRING)");
    ctx.execute("INSERT INTO users (id, name) VALUE (1, \"alice\")");
    ctx.execute("INSERT INTO users (id, name) VALUE (2, \"bob\")");
    ctx.clearOutput();
    
    ctx.execute("DELETE FROM users");
    
    Database* db = ctx.system.getCurrentDatabase();
    Table* table = db->getTable("users");
    
    check(stats, table->findRowById(1)->deleted, "Row 1 deleted");
    check(stats, table->findRowById(2)->deleted, "Row 2 deleted");
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("2 rows deleted") != std::string::npos,
        "Two rows deleted"
    );
}

void test_executor_execute_select_with_indexed_lookup(TestStats& stats) {
    test_header("Execute SELECT with indexed lookup");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.execute("CREATE TABLE testdb.users (id INT INDEXED, name STRING)");
    ctx.execute("INSERT INTO users (id, name) VALUE (1, \"alice\")");
    ctx.execute("INSERT INTO users (id, name) VALUE (2, \"bob\")");
    ctx.clearOutput();
    
    ctx.execute("SELECT * FROM users WHERE id == 1");
    
    std::string out = ctx.getOutput();
    check(stats, out.find("\"id\": 1") != std::string::npos, "Found by index");
    check(
        stats,
        out.find("\"id\": 2") == std::string::npos,
        "Other row not found"
    );
}

void test_executor_execute_between_condition(TestStats& stats) {
    test_header("Execute SELECT with BETWEEN");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.execute("CREATE TABLE testdb.users (id INT, age INT)");
    ctx.execute("INSERT INTO users (id, age) VALUE (1, 20)");
    ctx.execute("INSERT INTO users (id, age) VALUE (2, 25)");
    ctx.execute("INSERT INTO users (id, age) VALUE (3, 30)");
    ctx.execute("INSERT INTO users (id, age) VALUE (4, 35)");
    ctx.clearOutput();
    
    ctx.execute("SELECT * FROM users WHERE age BETWEEN 25 AND 35");
    
    std::string out = ctx.getOutput();
    check(stats, out.find("\"id\": 2") != std::string::npos, "Age 25 included");
    check(stats, out.find("\"id\": 3") != std::string::npos, "Age 30 included");
    check(
        stats,
        out.find("\"id\": 4") == std::string::npos,
        "Age 35 excluded (upper bound exclusive)"
    );
    check(stats, out.find("\"id\": 1") == std::string::npos, "Age 20 excluded");
}

void test_executor_execute_select_deleted_rows_excluded(TestStats& stats) {
    test_header("Execute SELECT excludes deleted rows");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.execute("CREATE TABLE testdb.users (id INT, name STRING)");
    ctx.execute("INSERT INTO users (id, name) VALUE (1, \"alice\")");
    ctx.execute("INSERT INTO users (id, name) VALUE (2, \"bob\")");
    ctx.execute("DELETE FROM users WHERE id == 2");
    ctx.clearOutput();
    
    ctx.execute("SELECT * FROM users");
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("\"id\": 1") != std::string::npos,
        "Active row present"
    );
    check(
        stats,
        out.find("\"id\": 2") == std::string::npos,
        "Deleted row excluded"
    );
}

void test_executor_execute_update_deleted_rows_excluded(TestStats& stats) {
    test_header("Execute UPDATE excludes deleted rows");
    
    TestExecutorContext ctx;
    ctx.execute("CREATE DATABASE testdb");
    ctx.execute("USE testdb");
    ctx.execute("CREATE TABLE testdb.users (id INT, name STRING)");
    ctx.execute("INSERT INTO users (id, name) VALUE (1, \"alice\")");
    ctx.execute("INSERT INTO users (id, name) VALUE (2, \"bob\")");
    ctx.execute("DELETE FROM users WHERE id == 2");
    ctx.clearOutput();
    
    ctx.execute("UPDATE users SET name = \"changed\"");
    
    Database* db = ctx.system.getCurrentDatabase();
    Table* table = db->getTable("users");
    check(
        stats,
        table->findRowById(1)->values[1].asString() == "changed",
        "Active row updated"
    );
    check(
        stats,
        table->findRowById(2)->values[1].asString() == "bob",
        "Deleted row unchanged"
    );
}

void test_executor_execute_unknown_command(TestStats& stats) {
    test_header("Execute unknown command");
    
    ScopedDataDirectory guard;
    System system;
    Executor executor(system);
    
    bool caught = false;
    try {
        Command cmd;
        cmd.type = CommandType::kUnknown;
        executor.execute(cmd);
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "Unknown command throws error");
}

int main() {
    TestStats stats;
    std::cout << "Running Executor tests..." << std::endl;
    
    test_executor_execute_create_database(stats);
    test_executor_execute_drop_database(stats);
    test_executor_execute_drop_nonexistent_database(stats);
    test_executor_execute_use_database(stats);
    test_executor_execute_create_table(stats);
    test_executor_execute_create_table_no_database(stats);
    test_executor_execute_drop_table(stats);
    test_executor_execute_insert(stats);
    test_executor_execute_insert_multiple_rows(stats);
    test_executor_execute_insert_no_table(stats);
    test_executor_execute_select(stats);
    test_executor_execute_select_all(stats);
    test_executor_execute_select_with_where(stats);
    test_executor_execute_select_with_alias(stats);
    test_executor_execute_select_no_table(stats);
    test_executor_execute_update(stats);
    test_executor_execute_update_multiple_rows(stats);
    test_executor_execute_delete(stats);
    test_executor_execute_delete_all(stats);
    test_executor_execute_select_with_indexed_lookup(stats);
    test_executor_execute_between_condition(stats);
    test_executor_execute_select_deleted_rows_excluded(stats);
    test_executor_execute_update_deleted_rows_excluded(stats);
    test_executor_execute_unknown_command(stats);
    
    print_test_results(stats);
    if (stats.tests_failed > 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

