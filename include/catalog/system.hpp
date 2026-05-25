#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include "catalog/database.hpp"

namespace dbms {

class System {
public:
    System();

    void createDatabase(const std::string& name);
    Database* getDatabase(const std::string& name);
    void useDatabase(const std::string& name);
    
    Database* getCurrentDatabase();
    
    void dropDatabase(const std::string& name);

private:
    std::unordered_map<
        std::string,
        std::unique_ptr<Database>
    > databases;

    Database* current_database = nullptr;

    std::filesystem::path storage_root = "data";
};

} // namespace dbms

