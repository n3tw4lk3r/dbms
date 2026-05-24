#pragma once

#include <string>
#include <vector>

#include "sqlparser/command.hpp"

namespace dbms {

class Parser {
public:
    Command parse(const std::string& query);

private:
    std::vector<std::string> tokenize(const std::string& query);
    std::string normalize(const std::string& token);
    
    bool isValidIdentifier(const std::string& token) const;
    void validateIdentifier(const std::string& token) const;   
    
    bool isKeyword(const std::string& token) const;
    std::string normalizeKeyword(const std::string& token) const;
    std::string tryNormalizeToken(const std::string& token) const;

    Command parseCreate(const std::vector<std::string>& tokens);
    Command parseCreateTable(const std::vector<std::string>& tokens);

    Command parseUse(const std::vector<std::string>& tokens);
    Command parseDrop(const std::vector<std::string>& tokens);

    Command parseInsert(const std::vector<std::string>& tokens);
    Command parseSelect(const std::vector<std::string>& tokens);
    Command parseUpdate(const std::vector<std::string>& tokens);
    Command parseDelete(const std::vector<std::string>& tokens);

    Value parseValue(const std::string& token);
    std::vector<Condition> parseConditions(
        const std::vector<std::string>& tokens,
        size_t pos
    );
    Operand parseOperand(const std::string& token);
    void parseTableName(
        const std::string& fullName,
        std::string& database_name,
        std::string& table_name
    );
};

} // namespace dbms
