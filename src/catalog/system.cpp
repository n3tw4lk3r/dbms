#include "catalog/system.hpp"

namespace dbms {

void System::createDatabase(const std::string& name) {
    databases[name] = std::make_unique<Database>(name);
}

Database* System::getDatabase(const std::string& name) {
    auto it = databases.find(name);

    if (it == databases.end()) {
        return nullptr;
    }

    return it->second.get();
}

void System::useDatabase(const std::string& name) {
    Database* db = getDatabase(name);

    if (!db) {
        throw std::runtime_error("Database not found");
    }

    current_database = db;
}

Database* System::getCurrentDatabase() {
    return current_database;
}

void System::dropDatabase(const std::string& name) {
    auto it = databases.find(name);

    if (it == databases.end()) {
        return;
    }

    if (current_database == it->second.get()) {
        current_database = nullptr;
    }

    databases.erase(it);
}

} // namespace dbms
