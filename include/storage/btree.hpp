#pragma once

#include <memory>

#include "storage/btree_node.hpp"

namespace dbms {

class BTree {
public:
    explicit BTree(size_t min_degree = 3);

    void Insert(const IndexedValue& key, RowId row_id);
    bool Erase(const IndexedValue& key);
    bool Contains(const IndexedValue& key) const;
    RowId Find(const IndexedValue& key) const;
    
    size_t Size() const;
    bool Empty() const;
    
    bool Verify() const;

private:
    void InsertNonFull(BTreeNode* node, const BTreeEntry& entry);
    void SplitChild(BTreeNode* parent, size_t child_index);

    BTreeEntry* Search(BTreeNode* node, const IndexedValue& key) const;

    void EraseInternal(BTreeNode* node, const IndexedValue& key);
    void EraseFromLeaf(BTreeNode* node, size_t index);
    void EraseFromInternal(BTreeNode* node, size_t index);

    BTreeEntry GetPredecessor(BTreeNode* node);
    BTreeEntry GetSuccessor(BTreeNode* node);

    void FillChild(BTreeNode* node, size_t child_index);

    void BorrowFromPrevious(BTreeNode* node, size_t child_index);
    void BorrowFromNext(BTreeNode* node, size_t child_index);

    void MergeChildren(BTreeNode* node, size_t child_index);

    size_t FindKeyIndex(BTreeNode* node, const IndexedValue& key) const;
    
    bool VerifyNode(
        const BTreeNode* node,
        size_t depth,
        size_t& leaf_depth
    ) const;

private:
    size_t min_degree;

    std::unique_ptr<BTreeNode> root;
};

} // namespace dbms

