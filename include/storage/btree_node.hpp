#pragma once

#include <vector>

#include "storage/indexed_value.hpp"
#include "storage/row_id.hpp"

namespace dbms {

struct BTreeEntry {
    IndexedValue key;
    RowId row_id;
};

class BTreeNode {
public:
    explicit BTreeNode(bool is_leaf);

    bool is_leaf;

    std::vector<BTreeEntry> entries;
    std::vector<BTreeNode*> children;
};

} // namespace dbms

