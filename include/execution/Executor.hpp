#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "catalog/System.hpp"
#include "common/types.hpp"
#include "sqlparser/Command.hpp"
#include "storage/Row.hpp"

namespace dbms {

class Executor {
public:
    explicit Executor(System& system);

    void Execute(const Command& cmd);

private:
    void ExecuteCreateDatabase(const Command& cmd);
    void ExecuteDropDatabase(const Command& cmd);
    void ExecuteUseDatabase(const Command& cmd);
    void ExecuteCreateTable(const Command& cmd);
    void ExecuteDropTable(const Command& cmd);
    void ExecuteInsert(const Command& cmd);
    void ExecuteSelect(const Command& cmd);
    void ExecuteUpdate(const Command& cmd);
    void ExecuteDelete(const Command& cmd);

    Database* ResolveDatabase(const Command& cmd);

    void PrintJsonRows(
        const std::vector<const Row*>& rows,
        const Command& cmd,
        const std::vector<ColumnSchema>& schema
    );
    void PrintJsonRow(
        const Row& row,
        const Command& cmd,
        const std::vector<ColumnSchema>& schema
    );
    void PrintJsonValue(const Value& value);

    bool MatchConditions(
        const std::vector<Condition>& conditions,
        const Row& row,
        const std::vector<ColumnSchema>& schema
    );

    int FindColumnIndex(
        const std::vector<ColumnSchema>& schema,
        std::string_view name
    );

    void ValidateColumnExists(
        const std::vector<ColumnSchema>& schema,
        const std::string& column_name
    );
    void ValidateSelectColumns(
        const Command& cmd,
        const std::vector<ColumnSchema>& schema
    );
    void ValidateAssignments(
        const Command& cmd,
        const std::vector<ColumnSchema>& schema
    );
    void ValidateConditions(
        const std::vector<Condition>& conditions,
        const std::vector<ColumnSchema>& schema
    );

    Value ResolveOperand(
        const Operand& operand,
        const Row& row,
        const std::vector<ColumnSchema>& schema
    );

    bool LikeValues(
        const Value& value,
        const Value& pattern
    );

    bool CanUseIndexLookup(
        const Table& table,
        const std::vector<Condition>& conditions
    );

    Row* TryFindIndexedRow(
        Table& table,
        const std::vector<Condition>& conditions
    );

    void PrintSelectedRow(
        const Row& row,
        const Command& cmd,
        const std::vector<ColumnSchema>& schema
    );

    std::vector<Value> BuildInsertRow(
        const Table& table,
        const Command& cmd,
        const std::vector<Value>& values
    );


private:
    System& system;
};

} // namespace dbms

