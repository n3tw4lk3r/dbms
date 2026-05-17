#include "storage/btree.hpp"

#include <stdexcept>
#include <utility>

namespace dbms {

BTree::BTree(size_t min_degree) :
    min_degree(min_degree),
    root(std::make_unique<BTreeNode>(true))
{}

void BTree::insert(const IndexedValue& key, RowId row_id) {
    if (contains(key)) {
        throw std::runtime_error("Duplicate indexed key");
    }

    BTreeEntry entry;
    entry.key = key;
    entry.row_id = row_id;

    if (root->entries.size() == (min_degree << 1) - 1) {
        auto new_root = std::make_unique<BTreeNode>(false);

        new_root->children.push_back(std::move(root));

        splitChild(new_root.get(), 0);

        root = std::move(new_root);
    }

    insertNonFull(root.get(), entry);
}

bool BTree::erase(const IndexedValue& key) {
    if (!contains(key)) {
        return false;
    }

    eraseInternal(root.get(), key);

    if (!root->is_leaf && root->entries.empty()) {
        root = std::move(root->children[0]);
    }

    return true;
}

bool BTree::contains(const IndexedValue& key) const {
    return search(root.get(), key) != nullptr;
}

RowId BTree::find(const IndexedValue& key) const {
    BTreeEntry* entry = search(root.get(), key);

    if (!entry) {
        return 0;
    }

    return entry->row_id;
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

    if (node->children[i] ->entries.size() ==
        (min_degree << 1) - 1
    ) {
        splitChild(node, i);
        if (entry.key > node->entries[i].key) {
            ++i;
        }
    }

    insertNonFull(node->children[i].get(), entry);
}

void BTree::splitChild(BTreeNode* parent, size_t child_index) {
    BTreeNode* child = parent->children[child_index].get();
    auto sibling = std::make_unique<BTreeNode>(child->is_leaf);

    parent->entries.insert(
        parent->entries.begin() + child_index,
        child->entries[min_degree - 1]
    );

    for (size_t i = 0; i < min_degree - 1; ++i) {
        sibling->entries.push_back(
            child->entries[i + min_degree]
        );
    }

    child->entries.resize(min_degree - 1);

    if (!child->is_leaf) {
        for (size_t i = 0; i < min_degree; ++i) {
            sibling->children.push_back(
                std::move(child->children[i + min_degree])
            );
        }

        child->children.resize(min_degree);
    }

    parent->children.insert(
        parent->children.begin() + child_index + 1,
        std::move(sibling)
    );
}

BTreeEntry* BTree::search(
    BTreeNode* node,
    const IndexedValue& key
) const {
    size_t i = 0;

    while (i < node->entries.size() &&
        key > node->entries[i].key
    ) {
        ++i;
    }

    if (i < node->entries.size() && key == node->entries[i].key) {
        return &node->entries[i];
    }

    if (node->is_leaf) {
        return nullptr;
    }

    return search(node->children[i].get(), key);
}

size_t BTree::findKeyIndex(
    BTreeNode* node,
    const IndexedValue& key
) const {
    size_t index = 0;

    while (index < node->entries.size() &&
        node->entries[index].key < key
    ) {
        ++index;
    }

    return index;
}

void BTree::eraseInternal(
    BTreeNode* node,
    const IndexedValue& key
) {
    size_t index = findKeyIndex(node, key);

    bool key_found = index < node->entries.size() &&
        node->entries[index].key == key;

    if (key_found) {
        if (node->is_leaf) {
            eraseFromLeaf(node, index);
        } else {
            eraseFromInternal(node, index);
        }

        return;
    }

    if (node->is_leaf) {
        return;
    }

    bool last_child = index == node->entries.size();

    if (node->children[index]->entries.size() < min_degree) {
        fillChild(node, index);
    }

    if (last_child && index > node->entries.size()) {
        eraseInternal(node->children[index - 1].get(), key);
        return;
    }

    eraseInternal(node->children[index].get(), key);
}

void BTree::eraseFromLeaf(BTreeNode* node, size_t index) {
    node->entries.erase(node->entries.begin() + index);
}

void BTree::eraseFromInternal(BTreeNode* node, size_t index) {
    IndexedValue key = node->entries[index].key;

    BTreeNode* left_child = node->children[index].get();
    BTreeNode* right_child = node->children[index + 1].get();

    if (left_child->entries.size() >= min_degree) {
        BTreeEntry predecessor = getPredecessor(left_child);
        node->entries[index] = predecessor;
        eraseInternal(left_child, predecessor.key);
        return;
    }

    if (right_child->entries.size() >= min_degree) {
        BTreeEntry successor = getSuccessor(right_child);
        node->entries[index] = successor;

        eraseInternal(right_child, successor.key);
        return;
    }

    mergeChildren(node, index);
    eraseInternal(left_child, key);
}

BTreeEntry BTree::getPredecessor(BTreeNode* node) {
    while (!node->is_leaf) {
        node = node->children.back().get();
    }

    return node->entries.back();
}

BTreeEntry BTree::getSuccessor(BTreeNode* node) {
    while (!node->is_leaf) {
        node = node->children.front().get();
    }

    return node->entries.front();
}

void BTree::fillChild(BTreeNode* node, size_t child_index) {
    if (child_index > 0 && node->children[child_index - 1]
        ->entries.size() >= min_degree
    ) {
        borrowFromPrevious(node, child_index);
        return;
    }

    if (child_index < node->entries.size() &&
        node->children[child_index + 1]->entries.size() >=
        min_degree
    ) {

        borrowFromNext(node, child_index);
        return;
    }

    if (child_index < node->entries.size()) {
        mergeChildren(node, child_index);
        return;
    }

    mergeChildren(node, child_index - 1);
}

void BTree::borrowFromPrevious(
    BTreeNode* node,
    size_t child_index
) {
    BTreeNode* child = node->children[child_index].get();
    BTreeNode* sibling = node->children[child_index - 1].get();

    child->entries.insert(
        child->entries.begin(),
        node->entries[child_index - 1]
    );

    if (!child->is_leaf) {
        child->children.insert(
            child->children.begin(),
            std::move(sibling->children.back())
        );

        sibling->children.pop_back();
    }

    node->entries[child_index - 1] = sibling->entries.back();
    sibling->entries.pop_back();
}

void BTree::borrowFromNext(BTreeNode* node, size_t child_index) {
    BTreeNode* child = node->children[child_index].get();
    BTreeNode* sibling = node->children[child_index + 1].get();

    child->entries.push_back(node->entries[child_index]);

    if (!child->is_leaf) {
        child->children.push_back(
            std::move(sibling->children.front())
        );

        sibling->children.erase(sibling->children.begin());
    }

    node->entries[child_index] = sibling->entries.front();

    sibling->entries.erase(sibling->entries.begin());
}

void BTree::mergeChildren(BTreeNode* node, size_t child_index) {
    BTreeNode* child = node->children[child_index].get();
    auto sibling = std::move(node->children[child_index + 1]);

    child->entries.push_back(node->entries[child_index]);

    for (const auto& entry : sibling->entries) {
        child->entries.push_back(entry);
    }

    if (!child->is_leaf) {
        for (auto& sibling_child : sibling->children) {
            child->children.push_back(std::move(sibling_child));
        }
    }

    node->entries.erase(node->entries.begin() + child_index);

    node->children.erase(node->children.begin() +
        child_index + 1
    );
}

} // namespace dbms

