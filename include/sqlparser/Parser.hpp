#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "sqlparser/Command.hpp"

namespace dbms {

class Parser {
public:
    Command Parse(const std::string& query);

private:
    std::vector<std::string> Tokenize(std::string_view query);
    std::string Normalize(const std::string& token);

    void ValidateIdentifier(const std::string& token) const;

    std::string NormalizeKeyword(const std::string& token) const;
    std::string TryNormalizeToken(const std::string& token) const;

    Command ParseCreate(const std::vector<std::string>& tokens);
    Command ParseCreateTable(const std::vector<std::string>& tokens);
    Command ParseUse(const std::vector<std::string>& tokens);
    Command ParseDrop(const std::vector<std::string>& tokens);
    Command ParseInsert(const std::vector<std::string>& tokens);
    Command ParseSelect(const std::vector<std::string>& tokens);
    Command ParseUpdate(const std::vector<std::string>& tokens);
    Command ParseDelete(const std::vector<std::string>& tokens);
    Value ParseValue(const std::string& token);
    std::vector<Condition> ParseConditions(
        const std::vector<std::string>& tokens,
        size_t pos
    );
    Operand ParseOperand(const std::string& token);
    void ParseTableName(
        const std::string& full_name,
        std::string& database_name,
        std::string& table_name
    );

    bool IsKeyword(const std::string& token) const;
    bool IsValidIdentifier(const std::string& token) const;
    bool IsSpace(const char ch) const;
    bool IsDigit(const char ch) const;
    bool IsAlpha(const char ch) const;
    bool IsAlnum(const char ch) const;
    bool IsOperatorToken(const std::string& token) const;

    char ToUpper(const char ch) const;

private:
    static constexpr size_t kMaxIdentifierLength = 128;
    static constexpr size_t kMaxStringLength = 4096;
    static constexpr size_t kMaxTokens = 10000;

};

} // namespace dbms
