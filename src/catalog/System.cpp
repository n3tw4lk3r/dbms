#include "catalog/System.hpp"

#include "exceptions/errors.hpp"

namespace dbms {

System::System() {
    std::filesystem::create_directories(storage_root);

    for (const auto& entry :
            std::filesystem::directory_iterator(storage_root)
    ) {
        if (!entry.is_directory()) {
            continue;
        }

        std::string database_name = entry.path().filename().string();

        databases[database_name] = std::make_unique<Database>(
            database_name,
            entry.path()
        );
    }
}

void System::CreateDatabase(const std::string& name) {
    if (databases.contains(name)) {
        throw DuplicateError("Database already exists");
    }

    std::filesystem::path database_path = storage_root / name;

    std::filesystem::create_directories(database_path);

    databases[name] = std::make_unique<Database>(
        name,
        database_path
    );
}

Database* System::GetDatabase(const std::string& name) {
    auto it = databases.find(name);

    if (it == databases.end()) {
        return nullptr;
    }

    return it->second.get();
}

void System::UseDatabase(const std::string& name) {
    Database* db = GetDatabase(name);

    if (!db) {
        throw NotFoundError("Database not found");
    }

    current_database = db;
}

Database* System::GetCurrentDatabase() {
    return current_database;
}

void System::DropDatabase(const std::string& name) {
    auto it = databases.find(name);

    if (it == databases.end()) {
        return;
    }

    if (current_database == it->second.get()) {
        current_database = nullptr;
    }

    std::filesystem::remove_all(storage_root / name);
    databases.erase(it);
}

} // namespace dbms

