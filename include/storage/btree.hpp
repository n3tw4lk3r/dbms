#pragma once

#include "storage/btree_node.hpp"

namespace dbms {

class BTree {
public:
    explicit BTree(size_t min_degree = 3);

    void insert(
        const IndexedValue& key,
        RowId row_id
    );

    bool contains(const IndexedValue& key) const;

    RowId find(const IndexedValue& key) const;

private:
    void insertNonFull(
        BTreeNode* node,
        const BTreeEntry& entry
    );

    void splitChild(
        BTreeNode* parent,
        size_t child_index
    );

    const BTreeEntry* search(
        BTreeNode* node,
        const IndexedValue& key
    ) const;

private:
    size_t min_degree;
    BTreeNode* root;
};

} // namespace dbms

