#pragma once

#include <memory>

#include "storage/btree_node.hpp"

namespace dbms {

class BTree {
public:
    explicit BTree(size_t min_degree = 3);

    void insert(const IndexedValue& key, RowId row_id);
    bool erase(const IndexedValue& key);
    bool contains(const IndexedValue& key) const;
    RowId find(const IndexedValue& key) const;
    
    size_t size() const;
    bool empty() const;
    
    bool verify() const;

private:
    void insertNonFull(BTreeNode* node, const BTreeEntry& entry);
    void splitChild(BTreeNode* parent, size_t child_index);

    BTreeEntry* search(BTreeNode* node, const IndexedValue& key) const;

    void eraseInternal(BTreeNode* node, const IndexedValue& key);
    void eraseFromLeaf(BTreeNode* node, size_t index);
    void eraseFromInternal(BTreeNode* node, size_t index);

    BTreeEntry getPredecessor(BTreeNode* node);
    BTreeEntry getSuccessor(BTreeNode* node);

    void fillChild(BTreeNode* node, size_t child_index);

    void borrowFromPrevious(BTreeNode* node, size_t child_index);
    void borrowFromNext(BTreeNode* node, size_t child_index);

    void mergeChildren(BTreeNode* node, size_t child_index);

    size_t findKeyIndex(BTreeNode* node, const IndexedValue& key) const;
    
    bool verifyNode(
        const BTreeNode* node,
        size_t depth,
        size_t& leaf_depth
    ) const;

private:
    size_t min_degree;

    std::unique_ptr<BTreeNode> root;
};

} // namespace dbms

