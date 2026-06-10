#include "catalog/database.hpp"

#include <unordered_set>

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

const std::string Database::GetName() const {
    return name;
}

void Database::CreateTable(
    const std::string& table_name,
    const std::vector<ColumnSchema>& schema
) {
    std::filesystem::path table_path = storage_path / table_name;
 
    std::unordered_set<std::string> names;

    for (const auto& column : schema) {
        if (names.contains(column.name)) {
            throw DuplicateError(
                "Duplicate column: " +
                column.name
            );
        }

        names.insert(column.name);
    }

    if (tables.contains(table_name)) {
        throw DuplicateError("Table already exists");
    }
    
    tables[table_name] = std::make_unique<Table>(
        table_name,
        schema,
        table_path
    );

    tables[table_name]->SaveSchema();
}

Table* Database::GetTable(const std::string& table_name) {
    auto it = tables.find(table_name);

    if (it == tables.end()) {
        return nullptr;
    }

    return it->second.get();
}
void Database::DropTable(const std::string& table_name) {
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

