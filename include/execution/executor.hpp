#pragma once

#include <string>
#include <vector>

#include "catalog/system.hpp"
#include "common/types.hpp"
#include "sqlparser/command.hpp"
#include "storage/row.hpp"

namespace dbms {

class Executor {
public:
    explicit Executor(System& system);

    void execute(const Command& cmd);

private:
    void executeCreateDatabase(const Command& cmd);
    void executeDropDatabase(const Command& cmd);
    void executeUseDatabase(const Command& cmd);

    void executeCreateTable(const Command& cmd);
    void executeDropTable(const Command& cmd);

    void executeInsert(const Command& cmd);
    void executeSelect(const Command& cmd);
    void executeUpdate(const Command& cmd);
    void executeDelete(const Command& cmd);

    Database* resolveDatabase(const Command& cmd);

    void printFullRow(const Row& row);

    void printSelectedColumns(
        const Row& row,
        const std::vector<ColumnSchema>& schema,
        const std::vector<SelectColumn>& columns
    );

    void printValue(const Value& value);

    bool matchConditions(
        const std::vector<Condition>& conditions,
        const Row& row,
        const std::vector<ColumnSchema>& schema
    );

    int findColumnIndex(
        const std::vector<ColumnSchema>& schema,
        const std::string& name
    );

    Value resolveOperand(
        const Operand& operand,
        const Row& row,
        const std::vector<ColumnSchema>& schema
    );

    bool likeValues(
        const Value& value,
        const Value& pattern
    );

    bool canUseIndexLookup(
        const Table& table,
        const std::vector<Condition>& conditions
    );

    Row* tryFindIndexedRow(
        Table& table,
        const std::vector<Condition>& conditions
    );

    void printSelectedRow(
        const Row& row,
        const Command& cmd,
        const std::vector<ColumnSchema>& schema
    );

private:
    System& system;
};

} // namespace dbms

