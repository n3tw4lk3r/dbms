#include <iostream>

#include "common/value.hpp"
#include "storage/btree.hpp"
#include "storage/indexed_value.hpp"
#include "exceptions/database_error.hpp"
#include "utils.hpp"

using namespace dbms;

void test_btree_create_empty(TestStats& stats) {
    test_header("Create empty B-tree");

    BTree tree;
    check(stats, tree.Size() == 0, "New tree has size 0");
    check(stats, tree.Empty(), "New tree is empty");
    check(stats, tree.Verify(), "New tree passes verification");
}

void test_btree_create_with_degree(TestStats& stats) {
    test_header("Create B-tree with custom min degree");

    BTree tree(4);
    check(stats, tree.Size() == 0, "Tree with custom degree has size 0");
    check(stats, tree.Empty(), "Tree with custom degree is empty");
    check(stats, tree.Verify(), "Tree with custom degree passes verification");
}

void test_btree_insert_single(TestStats& stats) {
    test_header("Insert single entry");

    BTree tree;
    tree.Insert(IndexedValue(Value(42)), 100);

    check(stats, tree.Size() == 1, "Tree size is 1 after insert");
    check(stats, !tree.Empty(), "Tree is not empty after insert");
    check(
        stats,
        tree.Contains(IndexedValue(Value(42))),
        "Tree contains inserted key"
    );
    check(
        stats,
        tree.Find(IndexedValue(Value(42))) == 100,
        "Find returns correct row_id"
    );
    check(stats, tree.Verify(), "Tree passes verification after insert");
}

void test_btree_insert_multiple_ascending(TestStats& stats) {
    test_header("Insert multiple entries in ascending order");

    BTree tree;
    for (int i = 1; i <= 20; ++i) {
        tree.Insert(IndexedValue(Value(i)), static_cast<RowId>(i * 10));
    }

    check(stats, tree.Size() == 20, "Tree size is 20");
    for (int i = 1; i <= 20; ++i) {
        check(stats, tree.Contains(IndexedValue(Value(i))), "Tree contains key");
    }
    check(
        stats,
        tree.Find(IndexedValue(Value(5))) == 50,
        "Find returns correct row_id"
    );
    check(
        stats,
        tree.Find(IndexedValue(Value(20))) == 200,
        "Find returns correct row_id"
    );
    check(stats, tree.Verify(), "Tree passes verification");
}

void test_btree_insert_multiple_descending(TestStats& stats) {
    test_header("Insert multiple entries in descending order");

    BTree tree;
    for (int i = 20; i >= 1; --i) {
        tree.Insert(IndexedValue(Value(i)), static_cast<RowId>(i * 10));
    }

    check(stats, tree.Size() == 20, "Tree size is 20");
    for (int i = 1; i <= 20; ++i) {
        check(stats, tree.Contains(IndexedValue(Value(i))), "Tree contains key");
    }
    check(stats, tree.Verify(), "Tree passes verification");
}

void test_btree_insert_duplicate(TestStats& stats) {
    test_header("Insert duplicate key throws exception");

    BTree tree;
    tree.Insert(IndexedValue(Value(10)), 100);

    bool threw = false;
    try {
        tree.Insert(IndexedValue(Value(10)), 200);
    } catch (const DuplicateError&) {
        threw = true;
    }

    check(stats, threw, "DuplicateError thrown on duplicate insert");
    check(stats, tree.Size() == 1, "Tree size unchanged after duplicate attempt");
    check(
        stats,
        tree.Find(IndexedValue(Value(10))) == 100,
        "Original entry preserved"
    );
}

void test_btree_find_non_existent(TestStats& stats) {
    test_header("Find non-existent key");

    BTree tree;
    tree.Insert(IndexedValue(Value(10)), 100);

    check(
        stats,
        tree.Find(IndexedValue(Value(99))) == 0,
        "Find returns 0 for non-existent key"
    );
    check(
        stats,
        !tree.Contains(IndexedValue(Value(99))),
        "Contains returns false for non-existent key"
    );
}

void test_btree_insert_string_keys(TestStats& stats) {
    test_header("Insert string keys");

    BTree tree;
    tree.Insert(IndexedValue(Value("apple")), 1);
    tree.Insert(IndexedValue(Value("banana")), 2);
    tree.Insert(IndexedValue(Value("cherry")), 3);
    tree.Insert(IndexedValue(Value("date")), 4);
    tree.Insert(IndexedValue(Value("elderberry")), 5);

    check(stats, tree.Size() == 5, "Tree size is 5 with string keys");
    check(stats, tree.Contains(IndexedValue(Value("apple"))), "Contains apple");
    check(stats, tree.Contains(IndexedValue(Value("banana"))), "Contains banana");
    check(stats, tree.Contains(IndexedValue(Value("cherry"))), "Contains cherry");
    check(
        stats,
        tree.Find(IndexedValue(Value("date"))) == 4,
        "Find returns correct row_id for date"
    );
    check(stats, tree.Verify(), "Tree passes verification with string keys");
}

void test_btree_erase_leaf(TestStats& stats) {
    test_header("Erase from leaf node");

    BTree tree;
    for (int i = 1; i <= 10; ++i) {
        tree.Insert(IndexedValue(Value(i)), static_cast<RowId>(i * 10));
    }

    check(
        stats,
        tree.Erase(IndexedValue(Value(5))),
        "Erase returns true for existing key"
    );
    check(stats, tree.Size() == 9, "Tree size decremented");
    check(
        stats,
        !tree.Contains(IndexedValue(Value(5))),
        "Tree no longer contains erased key"
    );
    check(
        stats,
        tree.Contains(IndexedValue(Value(4))),
        "Neighboring key still present"
    );
    check(
        stats,
        tree.Contains(IndexedValue(Value(6))),
        "Neighboring key still present"
    );
    check(stats, tree.Verify(), "Tree passes verification after erase");
}

void test_btree_erase_non_existent(TestStats& stats) {
    test_header("Erase non-existent key");

    BTree tree;
    tree.Insert(IndexedValue(Value(10)), 100);

    check(
        stats,
        !tree.Erase(IndexedValue(Value(99))),
        "Erase returns false for non-existent key"
    );
    check(stats, tree.Size() == 1, "Tree size unchanged");
}

void test_btree_erase_all(TestStats& stats) {
    test_header("Erase all entries");

    BTree tree;
    for (int i = 1; i <= 5; ++i) {
        tree.Insert(IndexedValue(Value(i)), static_cast<RowId>(i * 10));
    }

    for (int i = 1; i <= 5; ++i) {
        check(stats, tree.Erase(IndexedValue(Value(i))), "Erase successful");
    }

    check(stats, tree.Size() == 0, "Tree size is 0 after erasing all");
    check(stats, tree.Empty(), "Tree is empty after erasing all");
    check(stats, tree.Verify(), "Tree passes verification when empty");
}

void test_btree_large_insert(TestStats& stats) {
    test_header("Insert many entries to force splits");

    BTree tree(2);
    for (int i = 1; i <= 100; ++i) {
        tree.Insert(IndexedValue(Value(i)), static_cast<RowId>(i));
    }

    check(stats, tree.Size() == 100, "Tree size is 100");
    check(stats, tree.Verify(), "Tree passes verification after many inserts");

    for (int i = 1; i <= 100; ++i) {
        check(
            stats,
            tree.Contains(IndexedValue(Value(i))),
            "Tree contains all keys"
        );
    }
}

void test_btree_large_erase(TestStats& stats) {
    test_header("Erase many entries with underflow handling");

    BTree tree(2);
    for (int i = 1; i <= 50; ++i) {
        tree.Insert(IndexedValue(Value(i)), static_cast<RowId>(i));
    }

    for (int i = 1; i <= 25; ++i) {
        check(stats, tree.Erase(IndexedValue(Value(i))), "Erase successful");
    }

    check(stats, tree.Size() == 25, "Tree size is 25 after erasing half");
    check(stats, tree.Verify(), "Tree passes verification after many erases");

    for (int i = 26; i <= 50; ++i) {
        check(
            stats,
            tree.Contains(IndexedValue(Value(i))),
            "Remaining keys present"
        );
    }
}

void test_btree_insert_erase_alternating(TestStats& stats) {
    test_header("Alternating insert and erase");

    BTree tree;
    tree.Insert(IndexedValue(Value(1)), 10);
    tree.Erase(IndexedValue(Value(1)));
    check(stats, tree.Empty(), "Tree empty after insert-erase cycle");

    tree.Insert(IndexedValue(Value(2)), 20);
    tree.Insert(IndexedValue(Value(3)), 30);
    tree.Erase(IndexedValue(Value(2)));
    check(stats, tree.Size() == 1, "Tree size correct after partial erase");
    check(stats, tree.Contains(IndexedValue(Value(3))), "Remaining key present");

    tree.Insert(IndexedValue(Value(2)), 20);
    check(stats, tree.Size() == 2, "Tree size correct after re-insert");
    check(stats, tree.Verify(), "Tree passes verification");
}

void test_btree_empty_operations(TestStats& stats) {
    test_header("Operations on empty tree");

    BTree tree;
    check(
        stats,
        tree.Find(IndexedValue(Value(1))) == 0,
        "Find returns 0 on empty tree"
    );
    check(
        stats,
        !tree.Contains(IndexedValue(Value(1))),
        "Contains returns false on empty tree"
    );
    check(
        stats,
        !tree.Erase(IndexedValue(Value(1))),
        "Erase returns false on empty tree"
    );
}

int main() {
    TestStats stats;
    std::cout << "Running BTree tests..." << std::endl;

    test_btree_create_empty(stats);
    test_btree_create_with_degree(stats);
    test_btree_insert_single(stats);
    test_btree_insert_multiple_ascending(stats);
    test_btree_insert_multiple_descending(stats);
    test_btree_insert_duplicate(stats);
    test_btree_find_non_existent(stats);
    test_btree_insert_string_keys(stats);
    test_btree_erase_leaf(stats);
    test_btree_erase_non_existent(stats);
    test_btree_erase_all(stats);
    test_btree_large_insert(stats);
    test_btree_large_erase(stats);
    test_btree_insert_erase_alternating(stats);
    test_btree_empty_operations(stats);

    print_test_results(stats);
    if (stats.tests_failed > 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

