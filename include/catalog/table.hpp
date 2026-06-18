#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "common/types.hpp"
#include "common/value.hpp"
#include "sqlparser/command.hpp"
#include "storage/btree.hpp"
#include "storage/row.hpp"

namespace dbms {

class Table {
    friend class Database;

public:
    Table(
        std::string_view name,
        const std::vector<ColumnSchema>& schema,
        const std::filesystem::path& storage_path
    );

    Table(
        const std::filesystem::path& storage_path
    );

    const std::string GetName() const;

    RowId InsertRow(const std::vector<Value>& values);

    const std::vector<std::unique_ptr<Row>>& GetRows() const;
    std::vector<std::unique_ptr<Row>>& GetRowsMutable();
    const std::vector<ColumnSchema>& GetSchema() const;

    bool HasIndexedColumn(const std::string& column_name) const;

    bool ContainsIndexedValue(
        const std::string& column_name,
        const Value& value
    ) const;

    Row* FindByIndexedValue(
        const std::string& column_name,
        const Value& value
    );
    Row* FindRowById(RowId row_id);

    void UpdateRow(
        Row& row,
        const std::vector<Assignment>& assignments
    );

    void DeleteRow(RowId row_id);


private:
    void Save() const;
    void Load();

    void Compact();
    void TryCompact();

    void ValidateRow(const std::vector<Value>& values) const;
    void ValidateColumnCount(
        const std::vector<Value>& values
    ) const;
    void ValidateColumnType(
        const Value& value,
        const ColumnSchema& column
    ) const;
    void ValidateNotNull(
        const Value& value,
        const ColumnSchema& column
    ) const;
    void ValidateUniqueConstraints(
        const std::vector<Value>& values,
        RowId ignored_row_id = 0
    ) const;

    int FindColumnIndex(std::string_view column_name) const;

    void InsertIntoIndexes(const Row& row);
    void EraseFromIndexes(const Row& row);

    void AppendInsert(const Row& row);
    void AppendDelete(RowId row_id);
    void AppendUpdate(const Row& row);

    void SaveSchema() const;

private:
    std::string name;
    std::vector<ColumnSchema> schema;
    std::unordered_map<std::string, BTree> indexes;

    std::vector<std::unique_ptr<Row>> rows;
    std::unordered_map<RowId, Row*> row_lookup;
    RowId next_row_id = 1;

    std::filesystem::path storage_path;
    std::filesystem::path metadata_path;
    std::filesystem::path data_path;

    size_t operation_count = 0;
};

} // namespace dbms

