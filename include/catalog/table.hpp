#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "common/types.hpp"
#include "common/value.hpp"
#include "storage/btree.hpp"
#include "storage/row.hpp"
#include "sqlparser/command.hpp"

namespace dbms {

class Table {
public:
    Table(
        const std::string& name,
        const std::vector<ColumnSchema>& schema
    );

    const std::string getName() const;

    RowId insertRow(const std::vector<Value>& values);

    const std::vector<std::unique_ptr<Row>>& getRows() const;
    std::vector<std::unique_ptr<Row>>& getRowsMutable();

    const std::vector<ColumnSchema>& getSchema() const;

    bool hasIndexedColumn(const std::string& column_name) const;

    bool containsIndexedValue(
        const std::string& column_name,
        const Value& value
    ) const;

    Row* findByIndexedValue(
        const std::string& column_name,
        const Value& value
    );

    Row* findRowById(RowId row_id);

    void updateRow(
        Row& row,
        const std::vector<Assignment>& assignments
    );

    void deleteRow(Row& row);

private:
    void validateRow(const std::vector<Value>& values) const;

    void validateColumnCount(
        const std::vector<Value>& values
    ) const;

    void validateColumnType(
        const Value& value,
        const ColumnSchema& column
    ) const;

    void validateNotNull(
        const Value& value,
        const ColumnSchema& column
    ) const;

    void validateUniqueConstraints(
        const std::vector<Value>& values,
        RowId ignored_row_id = 0
    ) const;

    int findColumnIndex(const std::string& column_name) const;

    void insertIntoIndexes(const Row& row);
    void eraseFromIndexes(const Row& row);

private:
    RowId next_row_id = 1;
    std::string name;
    std::vector<ColumnSchema> schema;
    std::vector<std::unique_ptr<Row>> rows;
    std::unordered_map<std::string, BTree> indexes;
    std::unordered_map<RowId, Row*> row_lookup;
};

} // namespace dbms

