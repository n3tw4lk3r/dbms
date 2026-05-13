#include "storage/btree.hpp"

#include <stdexcept>

namespace dbms {

BTree::BTree(size_t min_degree) :
    min_degree(min_degree),
    root(new BTreeNode(true))
{}

void BTree::insert(
    const IndexedValue& key,
    RowId row_id
) {
    if (contains(key)) {
        throw std::runtime_error("Duplicate indexed key");
    }

    BTreeEntry entry;
    entry.key = key;
    entry.row_id = row_id;

    if (root->entries.size() == (min_degree << 1) - 1) {
        auto* new_root = new BTreeNode(false);
        new_root->children.push_back(root);
        splitChild(new_root, 0);
        root = new_root;
    }

    insertNonFull(root, entry);
}

void BTree::insertNonFull(
    BTreeNode* node,
    const BTreeEntry& entry
) {
    size_t i = node->entries.size();

    if (node->is_leaf) {
        node->entries.push_back(entry);

        while (i > 0 && entry.key < node->entries[i - 1].key) {
            node->entries[i] = node->entries[i - 1];
            --i;
        }

        node->entries[i] = entry;
        return;
    }

    while (i > 0 && entry.key < node->entries[i - 1].key) {
        --i;
    }

    if (node->children[i]->entries.size() == (min_degree << 1) - 1) {
        splitChild(node, i);
        if (entry.key > node->entries[i].key) {
            ++i;
        }
    }

    insertNonFull(node->children[i], entry);
}

void BTree::splitChild(
    BTreeNode* parent,
    size_t child_index
) {
    BTreeNode* child = parent->children[child_index];

    auto* sibling = new BTreeNode(child->is_leaf);

    parent->children.insert(parent->children.begin() + child_index + 1,
                            sibling);

    parent->entries.insert(parent->entries.begin() + child_index,
                           child->entries[min_degree - 1]);

    for (size_t i = 0; i < min_degree - 1; ++i) {
        sibling->entries.push_back(child->entries[i + min_degree]);
    }

    child->entries.resize(min_degree - 1);

    if (!child->is_leaf) {
        for (size_t i = 0; i < min_degree; ++i) {
            sibling->children.push_back(child->children[i + min_degree]);
        }

        child->children.resize(min_degree);
    }
}

bool BTree::contains(const IndexedValue& key) const {
    return search(root, key) != nullptr;
}

RowId BTree::find(const IndexedValue& key) const {
    const BTreeEntry* entry = search(root, key);

    if (!entry) {
        return 0;
    }

    return entry->row_id;
}

const BTreeEntry* BTree::search(
    BTreeNode* node,
    const IndexedValue& key
) const {
    size_t i = 0;

    while (i < node->entries.size() &&
           key > node->entries[i].key) {
        ++i;
    }

    if (i < node->entries.size() &&
        key == node->entries[i].key) {
        return &node->entries[i];
    }

    if (node->is_leaf) {
        return nullptr;
    }

    return search(node->children[i], key);
}

} // namespace dbms

