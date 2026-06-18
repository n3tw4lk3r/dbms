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

    std::string GetName() const;

    void CreateTable(
        const std::string& table_name,
        const std::vector<ColumnSchema>& schema
    );

    Table* GetTable(const std::string& table_name);
    void DropTable(const std::string& table_name);

private:
    std::string name;
    std::unordered_map<std::string, std::unique_ptr<Table>> tables;
    std::filesystem::path storage_path;
};

} // namespace dbms

