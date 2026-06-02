#include <iostream>
#include <memory>
#include <string>

#include "common/value.hpp"
#include "storage/btree_node.hpp"
#include "storage/indexed_value.hpp"
#include "utils.hpp"

using namespace dbms;

void test_btree_node_leaf_node(TestStats& stats) {
    test_header("Leaf node creation");
    
    BTreeNode node(true);
    check(stats, node.is_leaf, "Leaf node is_leaf is true");
    check(stats, node.entries.empty(), "Leaf node entries initially empty");
    check(stats, node.children.empty(), "Leaf node children initially empty");
}

void test_btree_node_internal_node(TestStats& stats) {
    test_header("Internal node creation");
    
    BTreeNode node(false);
    check(stats, !node.is_leaf, "Internal node is_leaf is false");
    check(stats, node.entries.empty(), "Internal node entries initially empty");
    check(stats, node.children.empty(), "Internal node children initially empty");
}

void test_btree_node_add_entries_to_leaf(TestStats& stats) {
    test_header("Add entries to leaf");
    
    BTreeNode node(true);
    BTreeEntry entry1;
    entry1.key = IndexedValue(Value(10));
    entry1.row_id = 100;
    node.entries.push_back(entry1);
    
    check(stats, node.entries.size() == 1, "Entry added");
    check(
        stats,
        node.entries[0].key == IndexedValue(Value(10)),
        "Entry key correct"
    );
    check(stats, node.entries[0].row_id == 100, "Entry row_id correct");
    
    BTreeEntry entry2;
    entry2.key = IndexedValue(Value(20));
    entry2.row_id = 200;
    node.entries.push_back(entry2);
    
    check(stats, node.entries.size() == 2, "Second entry added");
    check(
        stats,
        node.entries[1].key == IndexedValue(Value(20)),
        "Second entry key correct"
    );
}

void test_btree_node_add_entries_to_internal(TestStats& stats) {
    test_header("Add entries to internal node");
    
    BTreeNode node(false);
    BTreeEntry entry1;
    entry1.key = IndexedValue(Value("middle"));
    entry1.row_id = 1;
    node.entries.push_back(entry1);
    
    auto child1 = std::make_unique<BTreeNode>(true);
    auto child2 = std::make_unique<BTreeNode>(true);
    node.children.push_back(std::move(child1));
    node.children.push_back(std::move(child2));
    
    check(stats, node.entries.size() == 1, "Internal node has one entry");
    check(stats, node.children.size() == 2, "Internal node has two children");
}

void test_btree_node_multiple_entries(TestStats& stats) {
    test_header("Multiple entries");
    
    BTreeNode node(true);
    for (int i = 0; i < 100; ++i) {
        BTreeEntry entry;
        entry.key = IndexedValue(Value(i));
        entry.row_id = static_cast<RowId>(i * 10);
        node.entries.push_back(entry);
    }
    
    check(stats, node.entries.size() == 100, "100 entries added");
    check(
        stats,
        node.entries[0].key == IndexedValue(Value(0)),
        "First entry correct"
    );
    check(
        stats,
        node.entries[99].key == IndexedValue(Value(99)),
        "Last entry correct"
    );
    check(stats, node.entries[50].row_id == 500, "Middle entry row_id correct");
}

void test_btree_node_children_ownership(TestStats& stats) {
    test_header("Children ownership");
    
    BTreeNode parent(false);
    auto child = std::make_unique<BTreeNode>(true);
    BTreeNode* child_ptr = child.get();
    parent.children.push_back(std::move(child));
    
    check(stats, parent.children.size() == 1, "Child added to parent");
    check(
        stats,
        parent.children[0].get() == child_ptr,
        "Child pointer preserved"
    );
    check(stats, parent.children[0]->is_leaf, "Child is leaf");
}

void test_btree_node_string_keys(TestStats& stats) {
    test_header("String keys");
    
    BTreeNode node(true);
    BTreeEntry entry1;
    entry1.key = IndexedValue(Value("apple"));
    entry1.row_id = 1;
    node.entries.push_back(entry1);
    
    BTreeEntry entry2;
    entry2.key = IndexedValue(Value("banana"));
    entry2.row_id = 2;
    node.entries.push_back(entry2);
    
    BTreeEntry entry3;
    entry3.key = IndexedValue(Value("cherry"));
    entry3.row_id = 3;
    node.entries.push_back(entry3);
    
    check(stats, node.entries.size() == 3, "Three string entries");
    check(
        stats,
        node.entries[0].key == IndexedValue(Value("apple")),
        "Apple entry"
    );
    check(
        stats,
        node.entries[1].key == IndexedValue(Value("banana")),
        "Banana entry"
    );
    check(
        stats,
        node.entries[2].key == IndexedValue(Value("cherry")),
        "Cherry entry"
    );
}

void test_btree_node_null_keys(TestStats& stats) {
    test_header("Null keys");
    
    BTreeNode node(true);
    BTreeEntry entry;
    entry.key = IndexedValue(Value());
    entry.row_id = 1;
    node.entries.push_back(entry);
    
    check(stats, node.entries.size() == 1, "Null key entry added");
    check(stats, node.entries[0].key.getValue().isNull(), "Key is null");
    check(stats, node.entries[0].row_id == 1, "Row ID preserved with null key");
}

void test_btree_node_large_number_of_children(TestStats& stats) {
    test_header("Large number of children");
    
    BTreeNode parent(false);
    for (int i = 0; i < 50; ++i) {
        BTreeEntry entry;
        entry.key = IndexedValue(Value(i * 2));
        entry.row_id = static_cast<RowId>(i);
        parent.entries.push_back(entry);
        
        auto child = std::make_unique<BTreeNode>(true);
        parent.children.push_back(std::move(child));
    }
    auto last_child = std::make_unique<BTreeNode>(true);
    parent.children.push_back(std::move(last_child));
    
    check(stats, parent.entries.size() == 50, "50 entries added");
    check(stats, parent.children.size() == 51, "51 children (entries + 1)");
}

int main() {
    TestStats stats;
    std::cout << "Running BTreeNode tests..." << std::endl;
    
    test_btree_node_leaf_node(stats);
    test_btree_node_internal_node(stats);
    test_btree_node_add_entries_to_leaf(stats);
    test_btree_node_add_entries_to_internal(stats);
    test_btree_node_multiple_entries(stats);
    test_btree_node_children_ownership(stats);
    test_btree_node_string_keys(stats);
    test_btree_node_null_keys(stats);
    test_btree_node_large_number_of_children(stats);
    
    print_test_results(stats);
    if (stats.tests_failed > 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

