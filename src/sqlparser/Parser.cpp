#include "sqlparser/Parser.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <set>

#include "exceptions/errors.hpp"

namespace dbms {

Command Parser::Parse(const std::string& query) {
    if (query.empty()) {
        return {};
    }

    const auto tokens = Tokenize(query);
    if (tokens.empty()) {
        return {};
    }

    const std::string first = NormalizeKeyword(tokens[0]);

    if (first == "CREATE") {
        return ParseCreate(tokens);
    }

    if (first == "DROP") {
        return ParseDrop(tokens);
    }

    if (first == "USE") {
        return ParseUse(tokens);
    }

    if (first == "INSERT") {
        return ParseInsert(tokens);
    }

    if (first == "SELECT") {
        return ParseSelect(tokens);
    }

    if (first == "UPDATE") {
        return ParseUpdate(tokens);
    }

    if (first == "DELETE") {
        return ParseDelete(tokens);
    }

    throw ParserError("Unknown command: " + tokens[0]);
}

std::vector<std::string> Parser::Tokenize(std::string_view query) {
    std::vector<std::string> tokens;
    tokens.reserve(256);

    size_t i = 0;
    const size_t len = query.size();

    while (i < len) {
        if (IsSpace(query[i])) {
            ++i;
            continue;
        }

        if (i + 1 < len) {
            std::string two;
            two += query[i];
            two += query[i + 1];

            if (two == "==" || two == "!=" || two == "<=" || two == ">=") {
                tokens.push_back(std::move(two));
                i += 2;
                continue;
            }
        }

        if (
            query[i] == '=' ||
            query[i] == '<' ||
            query[i] == '>' ||
            query[i] == '(' ||
            query[i] == ')' ||
            query[i] == ',' ||
            query[i] == ';' ||
            query[i] == '*'
        ) {
            tokens.emplace_back(1, query[i]);
            ++i;
            continue;
        }

        if (query[i] == '"') {
            const size_t start = i;
            ++i;
            bool escaped = false;

            while (i < len) {
                if (escaped) {
                    escaped = false;
                } else if (query[i] == '\\') {
                    escaped = true;
                } else if (query[i] == '"') {
                    ++i;
                    break;
                }
                ++i;
            }

            tokens.emplace_back(query.substr(start, i - start));
            continue;
        }

        const size_t start = i;

        while (
            i < len &&
            !IsSpace(query[i]) &&
            query[i] != '(' &&
            query[i] != ')' &&
            query[i] != ',' &&
            query[i] != ';' &&
            query[i] != '"' &&
            query[i] != '*' &&
            query[i] != '=' &&
            query[i] != '<' &&
            query[i] != '>' &&
            query[i] != '!'
        ) {
            ++i;
        }

        if (i > start) {
            tokens.emplace_back(query.substr(start, i - start));
        }
    }

    if (tokens.size() > kMaxTokens) {
        throw ParserError("Query has too many tokens");
    }

    return tokens;
}

void Parser::ValidateIdentifier(const std::string& token) const {
    if (!IsValidIdentifier(token)) {
        throw ParserError("Invalid identifier: " + token);
    }

    if (IsKeyword(token)) {
        throw ParserError("Identifier cannot be a keyword: " + token);
    }
}

std::string Parser::NormalizeKeyword(const std::string& token) const {
    bool has_lower = false;
    bool has_upper = false;

    for (const char ch : token) {
        if (std::islower(static_cast<unsigned char>(ch))) {
            has_lower = true;
        } else if (std::isupper(static_cast<unsigned char>(ch))) {
            has_upper = true;
        }
    }

    if (has_lower && has_upper) {
        throw ParserError("Mixed-case keyword: " + token);
    }

    std::string result;
    result.reserve(token.size());
    for (const char ch : token) {
        result += ToUpper(ch);
    }
    return result;
}

Command Parser::ParseUse(const std::vector<std::string>& tokens) {
    if (tokens.size() != 2) {
        throw ParserError("USE requires exactly one database name");
    }

    ValidateIdentifier(tokens[1]);

    Command cmd;
    cmd.type = CommandType::kUseDatabase;
    cmd.database_name = tokens[1];
    return cmd;
}

Command Parser::ParseDrop(const std::vector<std::string>& tokens) {
    if (tokens.size() != 3) {
        throw ParserError("DROP syntax error");
    }

    Command cmd;
    const std::string target = NormalizeKeyword(tokens[1]);

    if (target == "DATABASE") {
        ValidateIdentifier(tokens[2]);
        cmd.type = CommandType::kDropDatabase;
        cmd.database_name = tokens[2];
        return cmd;
    }

    if (target == "TABLE") {
        cmd.type = CommandType::kDropTable;
        ParseTableName(tokens[2], cmd.database_name, cmd.table_name);
        return cmd;
    }

    throw ParserError("Expected DATABASE or TABLE after DROP");
}

Command Parser::ParseCreate(const std::vector<std::string>& tokens) {
    if (tokens.size() < 3) {
        throw ParserError("Incomplete CREATE statement");
    }

    const std::string target = NormalizeKeyword(tokens[1]);

    if (target == "DATABASE") {
        if (tokens.size() != 3) {
            throw ParserError("CREATE DATABASE syntax error");
        }

        ValidateIdentifier(tokens[2]);

        Command cmd;
        cmd.type = CommandType::kCreateDatabase;
        cmd.database_name = tokens[2];
        return cmd;
    }

    if (target == "TABLE") {
        return ParseCreateTable(tokens);
    }

    throw ParserError("Expected DATABASE or TABLE after CREATE");
}

Command Parser::ParseCreateTable(const std::vector<std::string>& tokens) {
    if (tokens.size() < 5) {
        throw ParserError("Incomplete CREATE TABLE statement");
    }

    Command cmd;
    cmd.type = CommandType::kCreateTable;

    ParseTableName(tokens[2], cmd.database_name, cmd.table_name);

    size_t pos = 3;

    if (tokens[pos] != "(") {
        throw ParserError("Expected '(' after table name");
    }

    ++pos;

    std::set<std::string> unique_columns;

    while (pos < tokens.size()) {
        if (tokens[pos] == ")") {
            ++pos;
            break;
        }

        if (pos + 1 >= tokens.size()) {
            throw ParserError("Incomplete column definition");
        }

        ColumnSchema column;
        column.name = tokens[pos];

        ValidateIdentifier(column.name);

        if (!unique_columns.insert(column.name).second) {
            throw ParserError("Duplicate column: " + column.name);
        }

        ++pos;

        const std::string& type = tokens[pos];
        if (type == "INT") {
            column.type = ColumnType::kInt;
        } else if (type == "STRING") {
            column.type = ColumnType::kString;
        } else {
            throw ParserError("Unknown column type: " + type);
        }

        ++pos;

        while (pos < tokens.size()) {
            if (tokens[pos] == "," || tokens[pos] == ")") {
                break;
            }

            if (tokens[pos] == "NOT_NULL") {
                column.not_null = true;
            } else if (tokens[pos] == "INDEXED") {
                column.indexed = true;
                column.not_null = true;
            } else {
                throw ParserError("Unknown column modifier: " + tokens[pos]);
            }

            ++pos;
        }

        cmd.columns.push_back(column);

        if (pos >= tokens.size()) {
            throw ParserError("Unexpected end of CREATE TABLE");
        }

        if (tokens[pos] == ",") {
            ++pos;
            continue;
        }

        if (tokens[pos] == ")") {
            ++pos;
            break;
        }

        throw ParserError("Expected ',' or ')'");
    }

    if (cmd.columns.empty()) {
        throw ParserError("Table must contain at least one column");
    }

    if (pos != tokens.size()) {
        throw ParserError("Unexpected tokens after CREATE TABLE");
    }

    return cmd;
}

Command Parser::ParseInsert(const std::vector<std::string>& tokens) {
    if (tokens.size() < 6) {
        throw ParserError("Incomplete INSERT statement");
    }

    if (NormalizeKeyword(tokens[1]) != "INTO") {
        throw ParserError("Expected INTO");
    }

    Command cmd;
    cmd.type = CommandType::kInsert;

    size_t pos = 2;
    ParseTableName(tokens[pos], cmd.database_name, cmd.table_name);
    ++pos;

    if (pos >= tokens.size()) {
        throw ParserError("Unexpected end after table name");
    }

    if (tokens[pos] == "(") {
        ++pos;
        std::set<std::string> unique_columns;
        bool expect_identifier = true;

        while (pos < tokens.size()) {
            if (tokens[pos] == ")") {
                ++pos;
                break;
            }

            if (expect_identifier) {
                ValidateIdentifier(tokens[pos]);

                if (!unique_columns.insert(tokens[pos]).second) {
                    throw ParserError("Duplicate column name: " + tokens[pos]);
                }

                cmd.column_names.push_back(tokens[pos]);
                expect_identifier = false;
            } else {
                if (tokens[pos] != ",") {
                    throw ParserError("Expected ',' between columns");
                }
                expect_identifier = true;
            }
            ++pos;
        }

        if (expect_identifier) {
            throw ParserError("Trailing comma in column list");
        }
    }

    if (pos >= tokens.size() || NormalizeKeyword(tokens[pos]) != "VALUE") {
        throw ParserError("Expected VALUE");
    }

    ++pos;

    while (pos < tokens.size()) {
        if (tokens[pos] != "(") {
            throw ParserError("Expected '(' in VALUE tuple");
        }

        ++pos;

        std::vector<Value> row;
        bool expect_value = true;

        while (pos < tokens.size()) {
            if (tokens[pos] == ")") {
                ++pos;
                break;
            }

            if (expect_value) {
                row.push_back(ParseValue(tokens[pos]));
                expect_value = false;
            } else {
                if (tokens[pos] != ",") {
                    throw ParserError("Expected ',' between values");
                }

                expect_value = true;
            }

            ++pos;
        }

        if (expect_value) {
            throw ParserError("Trailing comma in VALUE tuple");
        }

        if (!cmd.column_names.empty() && row.size() != cmd.column_names.size()) {
            throw ParserError("Column/value count mismatch");
        }

        cmd.values.push_back(std::move(row));

        if (pos >= tokens.size()) {
            break;
        }

        if (tokens[pos] != ",") {
            throw ParserError("Expected ',' between VALUE tuples");
        }

        ++pos;
    }

    if (cmd.values.empty()) {
        throw ParserError("INSERT requires at least one row");
    }

    return cmd;
}

Command Parser::ParseSelect(const std::vector<std::string>& tokens) {
    Command cmd;
    cmd.type = CommandType::kSelect;

    if (tokens.size() < 4) {
        throw ParserError("Invalid SELECT statement");
    }

    size_t pos = 1;
    while (pos < tokens.size()) {
        const std::string upper = NormalizeKeyword(tokens[pos]);
        if (upper == "FROM") {
            break;
        }

        if (tokens[pos] == ",") {
            ++pos;
            continue;
        }

        SelectColumn col;
        col.name = tokens[pos];
        ++pos;

        if (pos < tokens.size()) {
            const std::string upper_next = NormalizeKeyword(tokens[pos]);

            if (upper_next == "AS") {
                ++pos;
                if (pos < tokens.size()) {
                    const std::string upper_alias = NormalizeKeyword(tokens[pos]);
                    if (upper_alias != "FROM" && tokens[pos] != ",") {
                        col.alias = tokens[pos];
                        ++pos;
                    }
                }
            } else if (
                upper_next != "FROM" &&
                tokens[pos] != "," &&
                !IsKeyword(tokens[pos])
            ) {
                col.alias = tokens[pos];
                ++pos;
            }
        }

        cmd.select_columns.push_back(col);
    }

    if (pos >= tokens.size() || NormalizeKeyword(tokens[pos]) != "FROM") {
        throw ParserError("Expected FROM in SELECT");
    }
    ++pos;

    if (pos >= tokens.size()) {
        throw ParserError("Expected table name after FROM");
    }

    ParseTableName(tokens[pos], cmd.database_name, cmd.table_name);
    ++pos;

    if (pos < tokens.size() && NormalizeKeyword(tokens[pos]) == "WHERE") {
        ++pos;
        cmd.conditions = ParseConditions(tokens, pos);
    }

    return cmd;
}

Command Parser::ParseUpdate(const std::vector<std::string>& tokens) {
    Command cmd;
    cmd.type = CommandType::kUpdate;

    if (tokens.size() < 6) {
        throw ParserError("Invalid UPDATE statement");
    }

    size_t pos = 1;
    ParseTableName(tokens[pos], cmd.database_name, cmd.table_name);
    ++pos;

    if (pos >= tokens.size() || NormalizeKeyword(tokens[pos]) != "SET") {
        throw ParserError("Expected SET in UPDATE");
    }
    ++pos;

    while (pos < tokens.size()) {
        const std::string upper = NormalizeKeyword(tokens[pos]);
        if (upper == "WHERE") {
            break;
        }

        if (tokens[pos] == ",") {
            ++pos;
            continue;
        }

        Assignment assignment;
        assignment.column = tokens[pos];
        ++pos;

        if (pos >= tokens.size() || (tokens[pos] != "=" && tokens[pos] != "==")) {
            throw ParserError("Expected = in SET assignment");
        }
        ++pos;

        if (pos >= tokens.size()) {
            throw ParserError("Expected value in SET assignment");
        }

        assignment.value = ParseValue(tokens[pos]);
        ++pos;
        cmd.assignments.push_back(assignment);
    }

    if (pos < tokens.size() && NormalizeKeyword(tokens[pos]) == "WHERE") {
        ++pos;
        cmd.conditions = ParseConditions(tokens, pos);
    }

    return cmd;
}

Command Parser::ParseDelete(const std::vector<std::string>& tokens) {
    Command cmd;
    cmd.type = CommandType::kDelete;

    if (tokens.size() < 3) {
        throw ParserError("Invalid DELETE statement");
    }

    size_t pos = 1;

    if (NormalizeKeyword(tokens[pos]) != "FROM") {
        throw ParserError("Expected FROM in DELETE");
    }
    ++pos;

    if (pos >= tokens.size()) {
        throw ParserError("Expected table name after FROM");
    }

    ParseTableName(tokens[pos], cmd.database_name, cmd.table_name);
    ++pos;

    if (pos < tokens.size() && NormalizeKeyword(tokens[pos]) == "WHERE") {
        ++pos;
        cmd.conditions = ParseConditions(tokens, pos);
    }

    return cmd;
}

std::vector<Condition> Parser::ParseConditions(
    const std::vector<std::string>& tokens,
    size_t pos
) {
    std::vector<Condition> conditions;

    while (pos < tokens.size()) {
        const std::string upper = NormalizeKeyword(tokens[pos]);
        if (upper == ";") {
            break;
        }

        if (upper == "AND" || upper == "OR") {
            ++pos;
            continue;
        }

        Condition cond;
        cond.lhs = ParseOperand(tokens[pos]);

        ++pos;

        if (pos >= tokens.size()) {
            throw ParserError("Expected operator in condition");
        }

        std::string op = tokens[pos];
        ++pos;

        if (!IsOperatorToken(op)) {
            throw ParserError("Invalid operator in condition: " + op);
        }

        cond.operator_type = op;

        if (pos >= tokens.size()) {
            throw ParserError("Expected right operand in condition");
        }

        if (op == "BETWEEN") {
            cond.rhs = ParseOperand(tokens[pos]);

            ++pos;
            if (pos >= tokens.size() || NormalizeKeyword(tokens[pos]) != "AND") {
                throw ParserError("Expected AND in BETWEEN condition");
            }

            ++pos;
            if (pos >= tokens.size()) {
                throw ParserError("Expected range end in BETWEEN condition");
            }

            cond.range_end = ParseOperand(tokens[pos]);

            ++pos;
        } else {
            cond.rhs = ParseOperand(tokens[pos]);
            ++pos;
        }

        conditions.push_back(cond);
    }

    return conditions;
}

Value Parser::ParseValue(const std::string& token) {
    if (token == "NULL") {
        return Value();
    }

    if (token.size() >= 2 && token.front() == '"' && token.back() == '"') {
        return Value(token.substr(1, token.size() - 2));
    }

    bool is_number = true;
    for (size_t i = 0; i < token.size(); ++i) {
        if (i == 0 && token[i] == '-') {
            continue;
        }

        if (!IsDigit(token[i])) {
            is_number = false;
            break;
        }
    }

    if (is_number && !token.empty() && token != "-") {
        int value = 0;
        const auto result = std::from_chars(
            token.data(),
            token.data() + token.size(),
            value
        );

        if (result.ec == std::errc::result_out_of_range) {
            throw ParserError("Integer overflow: " + token);
        }

        if (result.ec != std::errc() || result.ptr != token.data() + token.size()) {
            throw ParserError("Invalid integer: " + token);
        }

        return Value(value);
    }

    ValidateIdentifier(token);
    return Value(token);
}

Operand Parser::ParseOperand(const std::string& token) {
    Operand op;

    if (token == "NULL") {
        op.value = Value();
        return op;
    }

    if (token.size() >= 2 && token.front() == '"' && token.back() == '"') {
        op.value = Value(token.substr(1, token.size() - 2));
        return op;
    }

    bool is_number = true;
    for (size_t i = 0; i < token.size(); ++i) {
        if (i == 0 && token[i] == '-') {
            continue;
        }

        if (!IsDigit(token[i])) {
            is_number = false;
            break;
        }
    }

    if (is_number && !token.empty()) {
        int value = 0;
        const auto result = std::from_chars(
            token.data(),
            token.data() + token.size(),
            value
        );

        if (result.ec != std::errc() || result.ptr != token.data() + token.size()) {
            throw ParserError("Invalid integer: " + token);
        }

        op.value = Value(value);
        return op;
    }

    ValidateIdentifier(token);
    op.is_column = true;
    op.column = token;
    return op;
}

void Parser::ParseTableName(
    const std::string& full_name,
    std::string& database_name,
    std::string& table_name
) {
    if (full_name.empty()) {
        throw ParserError("Empty table name");
    }

    const size_t dot = full_name.find('.');

    if (dot == std::string::npos) {
        ValidateIdentifier(full_name);
        database_name.clear();
        table_name = full_name;
        return;
    }

    if (full_name.find('.', dot + 1) != std::string::npos) {
        throw ParserError("Too many dots in table name");
    }

    if (dot == 0 || dot == full_name.size() - 1) {
        throw ParserError("Malformed qualified table name");
    }

    database_name = full_name.substr(0, dot);
    table_name = full_name.substr(dot + 1);

    ValidateIdentifier(database_name);
    ValidateIdentifier(table_name);
}

bool Parser::IsValidIdentifier(const std::string& token) const {
    if (token.empty() || token.size() > kMaxIdentifierLength) {
        return false;
    }

    if (IsDigit(token.front())) {
        return false;
    }

    for (const char ch : token) {
        if (!IsAlnum(ch) && ch != '_') {
            return false;
        }
    }
    return true;
}

bool Parser::IsKeyword(const std::string& token) const {
    static constexpr std::array keywords = {
        "CREATE",
        "DATABASE",
        "DROP",
        "USE",
        "TABLE",
        "INSERT",
        "INTO",
        "VALUE",
        "SELECT",
        "FROM",
        "WHERE",
        "UPDATE",
        "SET",
        "DELETE",
        "AS",
        "BETWEEN",
        "AND",
        "LIKE",
        "NOT_NULL",
        "INDEXED",
        "INT",
        "STRING",
        "NULL"
    };

    std::string upper;
    upper.reserve(token.size());

    for (const char ch : token) {
        upper += ToUpper(ch);
    }

    return std::ranges::find(keywords, upper) != keywords.end();
}

bool Parser::IsSpace(const char ch) const {
    return std::isspace(static_cast<unsigned char>(ch));
}

bool Parser::IsDigit(const char ch) const {
    return std::isdigit(static_cast<unsigned char>(ch));
}

bool Parser::IsAlnum(const char ch) const {
    return std::isalnum(static_cast<unsigned char>(ch));
}

bool Parser::IsOperatorToken(const std::string& token) const {
    static const std::set<std::string> operators = {
        "=",
        "==",
        "!=",
        "<",
        ">",
        "<=",
        ">=",
        "BETWEEN",
        "LIKE"
    };
    return operators.contains(token);
}

char Parser::ToUpper(const char ch) const {
    return static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
}

} // namespace dbms

