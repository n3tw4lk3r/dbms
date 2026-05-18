#include "catalog/database.hpp"

namespace dbms {

Database::Database(const std::string& name) :
    name(name)
{}

const std::string Database::getName() const {
    return name;
}

void Database::createTable(
    const std::string& table_name,
    const std::vector<ColumnSchema>& schema
) {
    tables[table_name] = std::make_unique<Table>(table_name, schema);
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

    tables.erase(it);
}

} // namespace dbms
