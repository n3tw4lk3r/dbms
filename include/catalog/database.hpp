#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include "catalog/table.hpp"

namespace dbms {

class Database {
public:
    Database(
        const std::string& name,
        const std::filesystem::path& storage_path
    );

    const std::string getName() const;
    
    void createTable(
        const std::string& table_name,
        const std::vector<ColumnSchema>& schema
    );
    
    Table* getTable(const std::string& table_name);
    void dropTable(const std::string& table_name);

private:
    std::string name;
    
    std::unordered_map<
        std::string,
        std::unique_ptr<Table>
    > tables;
    
    std::filesystem::path storage_path;
};

} // namespace dbms

