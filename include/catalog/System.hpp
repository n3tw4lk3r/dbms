#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include "catalog/Database.hpp"

namespace dbms {

class System {
public:
    System();

    void CreateDatabase(const std::string& name);
    Database* GetDatabase(const std::string& name);
    void UseDatabase(const std::string& name);

    Database* GetCurrentDatabase();

    void DropDatabase(const std::string& name);

private:
    std::unordered_map<std::string, std::unique_ptr<Database>> databases;
    Database* current_database = nullptr;
    std::filesystem::path storage_root = "data";
};

} // namespace dbms

