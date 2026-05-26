#include "catalog/database.hpp"

#include "exceptions/database_error.hpp"

namespace dbms {

Database::Database(
    const std::string& name,
    const std::filesystem::path& storage_path
) :
    name(name),
    storage_path(storage_path)
{
    std::filesystem::create_directories(storage_path);

    for (
        const auto& entry :
            std::filesystem::directory_iterator(storage_path)
    ) {
        if (!entry.is_regular_file()) {
            continue;
        }

        if (entry.path().extension() != ".meta") {
            continue;
        }

        std::string table_name = entry.path().stem().string();
        std::filesystem::path table_path = storage_path / table_name;

        tables[table_name] = std::make_unique<Table>(table_path);
    }
}

const std::string Database::getName() const {
    return name;
}

void Database::createTable(
    const std::string& table_name,
    const std::vector<ColumnSchema>& schema
) {
    std::filesystem::path table_path = storage_path / table_name;
    
    if (tables.contains(table_name)) {
        throw DatabaseError("Table already exists");
    }
    
    tables[table_name] = std::make_unique<Table>(
        table_name,
        schema,
        table_path
    );

    tables[table_name]->saveSchema();
}

Table* Database::getTable(const std::string& table_name) {
    auto it = tables.find(table_name);

    if (it == tables.end()) {
        return nullptr;
    }

    return it->second.get();
}
void Database::dropTable(const std::string& table_name) {
    auto it = tables.find(table_name);

    if (it == tables.end()) {
        return;
    }

    std::filesystem::remove(
        storage_path / (table_name + ".meta")
    );

    std::filesystem::remove(
        storage_path / (table_name + ".data")
    );

    tables.erase(it);
}

} // namespace dbms

