#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

#include "io/repl.hpp"
#include "utils.hpp"

using namespace dbms;
namespace fs = std::filesystem;

struct BatchTestContext {
    std::ostringstream output;
    std::streambuf* old_cout;
    std::ostringstream error_output;
    std::streambuf* old_cerr;
    
    BatchTestContext() {
        old_cout = std::cout.rdbuf(output.rdbuf());
        old_cerr = std::cerr.rdbuf(error_output.rdbuf());
    }
    
    ~BatchTestContext() {
        std::cout.rdbuf(old_cout);
        std::cerr.rdbuf(old_cerr);
    }
    
    std::string getOutput() const {
        return output.str();
    }
    
    std::string getError() const {
        return error_output.str();
    }
};

void run_batch_from_file(
    const std::string& content,
    TestStats& stats
) {
    std::string filename = "temp_batch_test.txt";
    
    {
        std::ofstream file(filename);
        check(stats, file.is_open(), "Temporary file created");
        file << content;
        file.close();
    }
    
    std::ifstream input(filename);
    check(stats, input.is_open(), "Temporary file opened for reading");
    run_batch_mode(input);
    input.close();
    
    std::remove(filename.c_str());
}

void test_batch_mode_exit_immediately(TestStats& stats) {
    test_header("Batch: exit immediately");
    
    ScopedDataDirectory guard;
    BatchTestContext ctx;
    
    std::istringstream input("exit\n");
    run_batch_mode(input);
    
    check(
        stats,
        ctx.getError().empty(),
        "No errors for immediate exit"
    );
    check(
        stats,
        true,
        "Function returned without hanging"
    );
}

void test_batch_mode_create_database(TestStats& stats) {
    test_header("Batch: CREATE DATABASE");
    
    ScopedDataDirectory guard;
    BatchTestContext ctx;
    
    std::string script = 
        "CREATE DATABASE batch_test;\n"
        "exit;\n";
    std::istringstream input(script);
    
    run_batch_mode(input);
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("Database batch_test created") != std::string::npos,
        "Database creation message printed"
    );
    check(
        stats,
        std::filesystem::exists("data/batch_test"),
        "Database directory exists after batch run"
    );
}

void test_batch_mode_create_and_use(TestStats& stats) {
    test_header("Batch: CREATE and USE database");
    
    ScopedDataDirectory guard;
    BatchTestContext ctx;
    
    std::string script = 
        "CREATE DATABASE batch_db;\n"
        "USE batch_db;\n"
        "exit;\n";
    std::istringstream input(script);
    
    run_batch_mode(input);
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("Database batch_db created") != std::string::npos,
        "DB creation message"
    );
    check(
        stats,
        out.find("Using database batch_db") != std::string::npos,
        "USE message printed"
    );
}

void test_batch_mode_create_table(TestStats& stats) {
    test_header("Batch: CREATE TABLE");
    
    ScopedDataDirectory guard;
    BatchTestContext ctx;
    
    std::string script = 
        "CREATE DATABASE batch_db;\n"
        "USE batch_db;\n"
        "CREATE TABLE batch_db.users (id INT INDEXED, name STRING NOT_NULL);\n"
        "exit;\n";
    std::istringstream input(script);
    
    run_batch_mode(input);
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("Table users created") != std::string::npos,
        "Table creation message printed"
    );
    check(
        stats,
        std::filesystem::exists("data/batch_db"),
        "Database directory exists"
    );
}

void test_batch_mode_insert_and_select(TestStats& stats) {
    test_header("Batch: INSERT and SELECT");
    
    ScopedDataDirectory guard;
    BatchTestContext ctx;
    
    std::string script = 
        "CREATE DATABASE batch_db;\n"
        "USE batch_db;\n"
        "CREATE TABLE batch_db.users (id INT INDEXED, name STRING);\n"
        "INSERT INTO users (id, name) VALUE (1, \"alice\");\n"
        "INSERT INTO users (id, name) VALUE (2, \"bob\");\n"
        "SELECT * FROM users;\n"
        "exit;\n";
    std::istringstream input(script);
    
    run_batch_mode(input);
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("1 rows inserted") != std::string::npos,
        "First insert message"
    );
    check(
        stats,
        out.find("\"id\": 1") != std::string::npos,
        "SELECT output contains id 1"
    );
    check(
        stats,
        out.find("\"name\": \"alice\"") != std::string::npos,
        "SELECT output contains alice"
    );
    check(
        stats,
        out.find("\"id\": 2") != std::string::npos,
        "SELECT output contains id 2"
    );
    check(
        stats,
        out.find("\"name\": \"bob\"") != std::string::npos,
        "SELECT output contains bob"
    );
}

void test_batch_mode_empty_input(TestStats& stats) {
    test_header("Batch: empty input");
    
    ScopedDataDirectory guard;
    BatchTestContext ctx;
    
    std::istringstream input("");
    run_batch_mode(input);
    
    check(
        stats,
        true,
        "Function handles empty input without hanging"
    );
}

void test_batch_mode_invalid_command(TestStats& stats) {
    test_header("Batch: invalid command");
    
    ScopedDataDirectory guard;
    BatchTestContext ctx;
    
    std::string script = 
        "INVALID COMMAND;\n"
        "exit;\n";
    std::istringstream input(script);
    
    run_batch_mode(input);
    
    std::string err = ctx.getError();
    check(
        stats,
        !err.empty(),
        "Error message printed for invalid command"
    );
}

void test_batch_mode_semicolon_separated_queries(TestStats& stats) {
    test_header("Batch: semicolon separated queries");
    
    ScopedDataDirectory guard;
    BatchTestContext ctx;
    
    std::string script = 
        "CREATE DATABASE batch_db; USE batch_db; CREATE TABLE batch_db.t (id INT); exit;\n";
    std::istringstream input(script);
    
    run_batch_mode(input);
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("Database batch_db created") != std::string::npos,
        "First query executed"
    );
    check(
        stats,
        out.find("Using database batch_db") != std::string::npos,
        "Second query executed"
    );
    check(
        stats,
        out.find("Table t created") != std::string::npos,
        "Third query executed"
    );
}

void test_batch_mode_multiline_query(TestStats& stats) {
    test_header("Batch: multiline query");
    
    ScopedDataDirectory guard;
    BatchTestContext ctx;
    
    std::string script = 
        "CREATE DATABASE batch_db;\n"
        "USE batch_db;\n"
        "CREATE TABLE batch_db.users (\n"
        "    id INT INDEXED,\n"
        "    name STRING NOT_NULL\n"
        ");\n"
        "INSERT INTO users (id, name) VALUE (1, \"alice\");\n"
        "exit;\n";
    std::istringstream input(script);
    
    run_batch_mode(input);
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("Table users created") != std::string::npos,
        "Multiline table creation executed"
    );
    check(
        stats,
        out.find("1 rows inserted") != std::string::npos,
        "Insert after multiline creation executed"
    );
}

void test_batch_mode_from_file_basic(TestStats& stats) {
    test_header("Batch from file: basic operations");
    
    ScopedDataDirectory guard;
    BatchTestContext ctx;
    
    std::string script = 
        "CREATE DATABASE testdb;\n"
        "USE testdb;\n"
        "CREATE TABLE testdb.users (id INT INDEXED, name STRING NOT_NULL);\n"
        "exit;\n";
    
    run_batch_from_file(script, stats);
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("Database testdb created") != std::string::npos,
        "Database creation message"
    );
    check(
        stats,
        out.find("Using database testdb") != std::string::npos,
        "USE message"
    );
    check(
        stats,
        out.find("Table users created") != std::string::npos,
        "Table creation message"
    );
    check(
        stats,
        fs::exists("data/testdb"),
        "Database directory created"
    );
}

void test_batch_mode_from_file_full_workflow(TestStats& stats) {
    test_header("Batch from file: full workflow");
    
    ScopedDataDirectory guard;
    BatchTestContext ctx;
    
    std::string script = 
        "CREATE DATABASE company;\n"
        "USE company;\n"
        "CREATE TABLE company.employees (id INT INDEXED, name STRING NOT_NULL, salary INT);\n"
        "INSERT INTO employees (id, name, salary) VALUE (1, \"Alice\", 50000);\n"
        "INSERT INTO employees (id, name, salary) VALUE (2, \"Bob\", 60000);\n"
        "INSERT INTO employees (id, name, salary) VALUE (3, \"Charlie\", 55000);\n"
        "SELECT id, name, salary FROM employees;\n"
        "UPDATE employees SET salary = 65000 WHERE id == 1;\n"
        "SELECT * FROM employees WHERE id == 1;\n"
        "DELETE FROM employees WHERE id == 3;\n"
        "SELECT * FROM employees;\n"
        "DROP TABLE company.employees;\n"
        "DROP DATABASE company;\n"
        "exit;\n";
    
    run_batch_from_file(script, stats);
    
    std::string out = ctx.getOutput();
    
    check(
        stats,
        out.find("Database company created") != std::string::npos,
        "Database created"
    );
    check(
        stats,
        out.find("Using database company") != std::string::npos,
        "Database used"
    );
    check(
        stats,
        out.find("Table employees created") != std::string::npos,
        "Table created"
    );
    check(
        stats,
        out.find("1 rows inserted") != std::string::npos,
        "Rows inserted"
    );
    check(
        stats,
        out.find("1 rows updated") != std::string::npos,
        "Row updated"
    );
    check(
        stats,
        out.find("1 rows deleted") != std::string::npos,
        "Row deleted"
    );
    check(
        stats,
        out.find("\"salary\": 65000") != std::string::npos,
        "Updated salary visible"
    );
    check(
        stats,
        out.find("Table dropped: employees") != std::string::npos,
        "Table dropped"
    );
    check(
        stats,
        out.find("Database dropped: company") != std::string::npos,
        "Database dropped"
    );
}

void test_batch_mode_from_file_multiline_query(TestStats& stats) {
    test_header("Batch from file: multiline query");
    
    ScopedDataDirectory guard;
    BatchTestContext ctx;
    
    std::string script = 
        "CREATE DATABASE testdb;\n"
        "USE testdb;\n"
        "CREATE TABLE testdb.products (\n"
        "    id INT INDEXED,\n"
        "    name STRING NOT_NULL,\n"
        "    price INT,\n"
        "    description STRING\n"
        ");\n"
        "INSERT INTO products (id, name, price, description)\n"
        "VALUE (1, \"Laptop\", 1000, \"High performance laptop\");\n"
        "SELECT * FROM products;\n"
        "exit;\n";
    
    run_batch_from_file(script, stats);
    
    std::string out = ctx.getOutput();
    check(
        stats,
        out.find("Table products created") != std::string::npos,
        "Multiline table created"
    );
    check(
        stats,
        out.find("1 rows inserted") != std::string::npos,
        "Row inserted"
    );
    check(
        stats,
        out.find("\"name\": \"Laptop\"") != std::string::npos,
        "Product data visible"
    );
    check(
        stats,
        out.find("\"price\": 1000") != std::string::npos,
        "Price visible"
    );
    check(
        stats,
        out.find("\"description\": \"High performance laptop\"") != std::string::npos,
        "Description visible"
    );
}

void test_batch_mode_from_file_with_errors(TestStats& stats) {
    test_header("Batch from file: handling errors");
    
    ScopedDataDirectory guard;
    BatchTestContext ctx;
    
    std::string script = 
        "CREATE DATABASE testdb;\n"
        "USE testdb;\n"
        "CREATE TABLE testdb.users (id INT INDEXED, name STRING);\n"
        "INSERT INTO nonexistent (id) VALUE (1);\n"
        "INSERT INTO users (id, name) VALUE (1, \"Alice\");\n"
        "SELECT * FROM users;\n"
        "exit;\n";
    
    run_batch_from_file(script, stats);
    
    std::string err = ctx.getError();
    std::string out = ctx.getOutput();
    
    check(
        stats,
        !err.empty(),
        "Error message for invalid table"
    );
    check(
        stats,
        out.find("1 rows inserted") != std::string::npos,
        "Valid insert still executed after error"
    );
    check(
        stats,
        out.find("\"name\": \"Alice\"") != std::string::npos,
        "Data from valid insert visible"
    );
}

void test_batch_mode_from_file_empty(TestStats& stats) {
    test_header("Batch from file: empty file");
    
    ScopedDataDirectory guard;
    BatchTestContext ctx;
    
    std::string script;
    
    run_batch_from_file(script, stats);
    
    check(
        stats,
        true,
        "Empty file handled without crash"
    );
}

int main() {
    TestStats stats;
    std::cout << "Running Batch Mode tests..." << std::endl;
    
    test_batch_mode_exit_immediately(stats);
    test_batch_mode_empty_input(stats);
    test_batch_mode_create_database(stats);
    test_batch_mode_create_and_use(stats);
    test_batch_mode_create_table(stats);
    test_batch_mode_insert_and_select(stats);
    test_batch_mode_invalid_command(stats);
    test_batch_mode_semicolon_separated_queries(stats);
    test_batch_mode_multiline_query(stats);
    test_batch_mode_from_file_basic(stats);
    test_batch_mode_from_file_full_workflow(stats);
    test_batch_mode_from_file_multiline_query(stats);
    test_batch_mode_from_file_with_errors(stats);
    test_batch_mode_from_file_empty(stats);
    
    print_test_results(stats);
    if (stats.tests_failed > 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

