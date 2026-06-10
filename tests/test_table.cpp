#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include "catalog/table.hpp"
#include "common/types.hpp"
#include "common/value.hpp"
#include "utils.hpp"

using namespace dbms;
namespace fs = std::filesystem;

void test_table_creation(TestStats& stats) {
    test_header("Table creation");
    
    fs::path test_path = "test_table_data/test_creation";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true},
        {"name", ColumnType::kString, false, false}
    };
    
    Table table("users", schema, test_path / "users");
    
    check(stats, table.GetName() == "users", "Table name correct");
    check(stats, table.GetSchema().size() == 2, "Schema size correct");
    check(stats, table.GetRows().empty(), "No rows initially");
    
    fs::remove_all(test_path);
}

void test_table_get_name(TestStats& stats) {
    test_header("Get table name");
    
    fs::path test_path = "test_table_data/test_name";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {{"id", ColumnType::kInt}};
    
    Table t1("first", schema, test_path / "first");
    check(stats, t1.GetName() == "first", "First table name");
    
    Table t2("second_table_123", schema, test_path / "second");
    check(stats, t2.GetName() == "second_table_123", "Second table name");
    
    fs::remove_all(test_path);
}

void test_table_insert_row(TestStats& stats) {
    test_header("Insert row");
    
    fs::path test_path = "test_table_data/test_insert";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true},
        {"name", ColumnType::kString, false, false}
    };
    
    Table table("users", schema, test_path / "users");
    
    RowId id = table.InsertRow({Value(1), Value("john")});
    check(stats, id == 1, "First row ID is 1");
    
    const auto& rows = table.GetRows();
    check(stats, rows.size() == 1, "One row in table");
    check(stats, !rows[0]->deleted, "Row is not deleted");
    check(stats, rows[0]->values[0].AsInt() == 1, "First value correct");
    check(
        stats,
        rows[0]->values[1].AsString() == "john",
        "Second value correct"
    );
    
    fs::remove_all(test_path);
}

void test_table_insert_multiple_rows(TestStats& stats) {
    test_header("Insert multiple rows");
    
    fs::path test_path = "test_table_data/test_insert_multi";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true},
        {"value", ColumnType::kString, false, false}
    };
    
    Table table("data", schema, test_path / "data");
    
    RowId id1 = table.InsertRow({Value(1), Value("first")});
    RowId id2 = table.InsertRow({Value(2), Value("second")});
    RowId id3 = table.InsertRow({Value(3), Value("third")});
    
    check(stats, id1 == 1, "First ID is 1");
    check(stats, id2 == 2, "Second ID is 2");
    check(stats, id3 == 3, "Third ID is 3");
    check(stats, table.GetRows().size() == 3, "Three rows in table");
    
    fs::remove_all(test_path);
}

void test_table_insert_null_values(TestStats& stats) {
    test_header("Insert NULL values");
    
    fs::path test_path = "test_table_data/test_insert_null";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true},
        {"description", ColumnType::kString, false, false}
    };
    
    Table table("items", schema, test_path / "items");
    
    RowId id = table.InsertRow({Value(1), Value()});
    check(stats, id == 1, "Row with NULL inserted");
    
    const auto& rows = table.GetRows();
    check(stats, rows[0]->values[1].IsNull(), "NULL value is null");
    
    fs::remove_all(test_path);
}

void test_table_insert_wrong_column_count(TestStats& stats) {
    test_header("Insert wrong column count");
    
    fs::path test_path = "test_table_data/test_insert_wrong_count";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true},
        {"name", ColumnType::kString, false, false}
    };
    
    Table table("users", schema, test_path / "users");
    
    bool caught = false;
    try {
        table.InsertRow({Value(1)});
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "Too few columns throws error");
    
    caught = false;
    try {
        table.InsertRow({Value(1), Value("name"), Value(3)});
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "Too many columns throws error");
    
    fs::remove_all(test_path);
}

void test_table_insert_wrong_type(TestStats& stats) {
    test_header("Insert wrong type");
    
    fs::path test_path = "test_table_data/test_insert_wrong_type";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true},
        {"name", ColumnType::kString, false, false}
    };
    
    Table table("users", schema, test_path / "users");
    
    bool caught = false;
    try {
        table.InsertRow({Value("not_int"), Value("name")});
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "Wrong type for int column throws error");
    
    caught = false;
    try {
        table.InsertRow({Value(1), Value(42)});
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "Wrong type for string column throws error");
    
    fs::remove_all(test_path);
}

void test_table_insert_not_null_violation(TestStats& stats) {
    test_header("Insert NOT_NULL violation");
    
    fs::path test_path = "test_table_data/test_insert_not_null";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true},
        {"name", ColumnType::kString, true, false}
    };
    
    Table table("users", schema, test_path / "users");
    
    bool caught = false;
    try {
        table.InsertRow({Value(1), Value()});
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "NULL for NOT_NULL column throws error");
    
    fs::remove_all(test_path);
}

void test_table_unique_constraint(TestStats& stats) {
    test_header("Unique constraint on indexed column");
    
    fs::path test_path = "test_table_data/test_unique";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true},
        {"name", ColumnType::kString, false, false}
    };
    
    Table table("users", schema, test_path / "users");
    
    table.InsertRow({Value(1), Value("first")});
    
    bool caught = false;
    try {
        table.InsertRow({Value(1), Value("second")});
    } catch (...) {
        caught = true;
    }
    check(stats, caught, "Duplicate indexed value throws error");
    
    fs::remove_all(test_path);
}

void test_table_find_row_by_id(TestStats& stats) {
    test_header("Find row by ID");
    
    fs::path test_path = "test_table_data/test_find_by_id";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {{"id", ColumnType::kInt}};
    Table table("data", schema, test_path / "data");
    
    RowId id1 = table.InsertRow({Value(10)});
    RowId id2 = table.InsertRow({Value(20)});
    RowId id3 = table.InsertRow({Value(30)});
    
    Row* row = table.FindRowById(id1);
    check(stats, row != nullptr, "Row 1 found");
    check(stats, row->values[0].AsInt() == 10, "Row 1 value correct");
    
    row = table.FindRowById(id2);
    check(stats, row != nullptr, "Row 2 found");
    check(stats, row->values[0].AsInt() == 20, "Row 2 value correct");
    
    row = table.FindRowById(id3);
    check(stats, row != nullptr, "Row 3 found");
    
    row = table.FindRowById(999);
    check(stats, row == nullptr, "Nonexistent ID returns nullptr");
    
    row = table.FindRowById(0);
    check(stats, row == nullptr, "ID 0 returns nullptr");
    
    fs::remove_all(test_path);
}

void test_table_find_by_indexed_value(TestStats& stats) {
    test_header("Find by indexed value");
    
    fs::path test_path = "test_table_data/test_find_indexed";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true},
        {"name", ColumnType::kString, false, false}
    };
    
    Table table("users", schema, test_path / "users");
    
    table.InsertRow({Value(1), Value("alice")});
    table.InsertRow({Value(2), Value("bob")});
    
    Row* row = table.FindByIndexedValue("id", Value(1));
    check(stats, row != nullptr, "Found by indexed value");
    check(stats, row->values[1].AsString() == "alice", "Correct row found");
    
    row = table.FindByIndexedValue("id", Value(2));
    check(stats, row != nullptr, "Second row found");
    check(stats, row->values[1].AsString() == "bob", "Second row value correct");
    
    row = table.FindByIndexedValue("id", Value(99));
    check(stats, row == nullptr, "Nonexistent indexed value returns nullptr");
    
    row = table.FindByIndexedValue("name", Value("alice"));
    check(stats, row == nullptr, "Non-indexed column returns nullptr");
    
    fs::remove_all(test_path);
}

void test_table_contains_indexed_value(TestStats& stats) {
    test_header("Contains indexed value");
    
    fs::path test_path = "test_table_data/test_contains_indexed";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true}
    };
    
    Table table("data", schema, test_path / "data");
    table.InsertRow({Value(100)});
    
    check(
        stats,
        table.ContainsIndexedValue("id", Value(100)),
        "Contains indexed value"
    );
    check(
        stats,
        !table.ContainsIndexedValue("id", Value(200)),
        "Does not contain non-existent value"
    );
    check(
        stats,
        !table.ContainsIndexedValue("nonexistent", Value(100)),
        "Non-indexed column returns false"
    );
    
    fs::remove_all(test_path);
}

void test_table_has_indexed_column(TestStats& stats) {
    test_header("Has indexed column");
    
    fs::path test_path = "test_table_data/test_has_indexed";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true},
        {"name", ColumnType::kString, false, false},
        {"email", ColumnType::kString, false, true}
    };
    
    Table table("users", schema, test_path / "users");
    
    check(stats, table.HasIndexedColumn("id"), "id is indexed");
    check(stats, table.HasIndexedColumn("email"), "email is indexed");
    check(stats, !table.HasIndexedColumn("name"), "name is not indexed");
    check(
        stats,
        !table.HasIndexedColumn("nonexistent"),
        "Nonexistent column returns false"
    );
    
    fs::remove_all(test_path);
}

void test_table_update_row(TestStats& stats) {
    test_header("Update row");
    
    fs::path test_path = "test_table_data/test_update";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true},
        {"name", ColumnType::kString, false, false},
        {"age", ColumnType::kInt, false, false}
    };
    
    Table table("users", schema, test_path / "users");
    
    table.InsertRow({Value(1), Value("alice"), Value(25)});
    
    std::vector<Assignment> assignments = {
        {"name", Value("alice_updated")},
        {"age", Value(26)}
    };
    
    Row* row = table.FindRowById(1);
    table.UpdateRow(*row, assignments);
    
    row = table.FindRowById(1);
    check(stats, row->values[1].AsString() == "alice_updated", "Name updated");
    check(stats, row->values[2].AsInt() == 26, "Age updated");
    check(stats, row->values[0].AsInt() == 1, "ID unchanged");
    
    fs::remove_all(test_path);
}

void test_table_update_row_with_indexed_column(TestStats& stats) {
    test_header("Update row with indexed column change");
    
    fs::path test_path = "test_table_data/test_update_indexed";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true},
        {"code", ColumnType::kString, false, true}
    };
    
    Table table("data", schema, test_path / "data");
    
    table.InsertRow({Value(1), Value("AAA")});
    table.InsertRow({Value(2), Value("BBB")});
    
    check(
        stats,
        table.ContainsIndexedValue("code", Value("AAA")),
        "AAA indexed before update"
    );
    
    Row* row = table.FindRowById(1);
    std::vector<Assignment> assignments = {{"code", Value("CCC")}};
    table.UpdateRow(*row, assignments);
    
    check(
        stats,
        !table.ContainsIndexedValue("code", Value("AAA")),
        "AAA removed from index"
    );
    check(
        stats,
        table.ContainsIndexedValue("code", Value("CCC")),
        "CCC added to index"
    );
    
    row = table.FindByIndexedValue("code", Value("CCC"));
    check(stats, row != nullptr, "Can find updated row by new indexed value");
    check(stats, row->values[0].AsInt() == 1, "Correct row ID");
    
    fs::remove_all(test_path);
}

void test_table_update_unique_constraint_violation(TestStats& stats) {
    test_header("Update unique constraint violation");
    
    fs::path test_path = "test_table_data/test_update_unique";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true},
        {"code", ColumnType::kString, false, true}
    };
    
    Table table("data", schema, test_path / "data");
    
    table.InsertRow({Value(1), Value("AAA")});
    table.InsertRow({Value(2), Value("BBB")});
    
    Row* row = table.FindRowById(1);
    std::vector<Assignment> assignments = {{"code", Value("BBB")}};
    
    bool caught = false;
    try {
        table.UpdateRow(*row, assignments);
    } catch (...) {
        caught = true;
    }
    check(
        stats,
        caught,
        "Unique constraint violation on update throws error"
    );
    
    row = table.FindRowById(1);
    check(
        stats,
        row->values[1].AsString() == "AAA",
        "Value rolled back after failed update"
    );
    
    fs::remove_all(test_path);
}

void test_table_delete_row(TestStats& stats) {
    test_header("Delete row");
    
    fs::path test_path = "test_table_data/test_delete";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"id", ColumnType::kInt, true, true}
    };
    
    Table table("data", schema, test_path / "data");
    
    table.InsertRow({Value(1)});
    table.InsertRow({Value(2)});
    table.InsertRow({Value(3)});
    
    check(stats, table.GetRows().size() == 3, "3 rows before delete");
    
    table.DeleteRow(2);
    
    const auto& rows = table.GetRows();
    check(stats, rows.size() == 3, "Row count unchanged (logical delete)");
    
    Row* row = table.FindRowById(2);
    check(stats, row != nullptr, "Deleted row still exists");
    check(stats, row->deleted, "Deleted row marked as deleted");
    
    row = table.FindRowById(1);
    check(stats, !row->deleted, "Other row not affected");
    
    row = table.FindRowById(3);
    check(stats, !row->deleted, "Other row not affected");
    
    check(
        stats,
        !table.ContainsIndexedValue("id", Value(2)),
        "Deleted row removed from index"
    );
    check(
        stats,
        table.ContainsIndexedValue("id", Value(1)),
        "Other row still in index"
    );
    check(
        stats,
        table.ContainsIndexedValue("id", Value(3)),
        "Other row still in index"
    );
    
    fs::remove_all(test_path);
}

void test_table_delete_nonexistent_row(TestStats& stats) {
    test_header("Delete nonexistent row");
    
    fs::path test_path = "test_table_data/test_delete_nonexist";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {{"id", ColumnType::kInt}};
    Table table("data", schema, test_path / "data");
    
    table.DeleteRow(999);
    check(stats, true, "Deleting nonexistent row does not throw");
    
    fs::remove_all(test_path);
}

void test_table_get_schema(TestStats& stats) {
    test_header("Get schema");
    
    fs::path test_path = "test_table_data/test_schema";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {
        {"col1", ColumnType::kInt, true, true},
        {"col2", ColumnType::kString, false, false},
        {"col3", ColumnType::kInt, true, false}
    };
    
    Table table("test", schema, test_path / "test");
    
    const auto& retrieved = table.GetSchema();
    check(stats, retrieved.size() == 3, "Schema size correct");
    check(stats, retrieved[0].name == "col1", "Column 1 name");
    check(stats, retrieved[0].type == ColumnType::kInt, "Column 1 type");
    check(stats, retrieved[0].not_null, "Column 1 not_null");
    check(stats, retrieved[0].indexed, "Column 1 indexed");
    check(stats, retrieved[2].name == "col3", "Column 3 name");
    check(stats, retrieved[2].type == ColumnType::kInt, "Column 3 type");
    check(stats, retrieved[2].not_null, "Column 3 not_null");
    check(stats, !retrieved[2].indexed, "Column 3 not indexed");
    
    fs::remove_all(test_path);
}

void test_table_get_rows_mutable(TestStats& stats) {
    test_header("Get rows mutable");
    
    fs::path test_path = "test_table_data/test_rows_mutable";
    fs::remove_all(test_path);
    fs::create_directories(test_path);
    
    std::vector<ColumnSchema> schema = {{"value", ColumnType::kInt}};
    Table table("data", schema, test_path / "data");
    
    table.InsertRow({Value(10)});
    
    auto& rows = table.GetRowsMutable();
    check(stats, rows.size() == 1, "Mutable rows accessible");
    check(stats, rows[0]->values[0].AsInt() == 10, "Mutable row value correct");
    
    fs::remove_all(test_path);
}

int main() {
    TestStats stats;
    std::cout << "Running Table tests..." << std::endl;
    
    test_table_creation(stats);
    test_table_get_name(stats);
    test_table_insert_row(stats);
    test_table_insert_multiple_rows(stats);
    test_table_insert_null_values(stats);
    test_table_insert_wrong_column_count(stats);
    test_table_insert_wrong_type(stats);
    test_table_insert_not_null_violation(stats);
    test_table_unique_constraint(stats);
    test_table_find_row_by_id(stats);
    test_table_find_by_indexed_value(stats);
    test_table_contains_indexed_value(stats);
    test_table_has_indexed_column(stats);
    test_table_update_row(stats);
    test_table_update_row_with_indexed_column(stats);
    test_table_update_unique_constraint_violation(stats);
    test_table_delete_row(stats);
    test_table_delete_nonexistent_row(stats);
    test_table_get_schema(stats);
    test_table_get_rows_mutable(stats);
    
    print_test_results(stats);
    if (stats.tests_failed > 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

