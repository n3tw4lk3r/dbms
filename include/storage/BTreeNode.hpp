#pragma once

#include <memory>
#include <vector>

#include "storage/IndexedValue.hpp"
#include "storage/RowId.hpp"

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
    std::vector<std::unique_ptr<BTreeNode>> children;
};

} // namespace dbms

