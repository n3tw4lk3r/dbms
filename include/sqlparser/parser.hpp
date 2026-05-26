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
    
    void validateIdentifier(const std::string& token) const;   
    
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

    bool isKeyword(const std::string& token) const;
    bool isValidIdentifier(const std::string& token) const;
    bool IsSpace(const char ch) const;
    bool IsDigit(const char ch) const;
    bool IsAlpha(const char ch) const;
    bool IsAlnum(const char ch) const;
    bool IsOperatorToken(const std::string& token) const;

    char toUpper(const char ch) const;

    void throwParseError(const std::string& message) const;

private:
    static constexpr size_t kMaxIdentifierLength = 128;
    static constexpr size_t kMaxStringLength = 4096;
    static constexpr size_t kMaxTokens = 10000;

};

} // namespace dbms
