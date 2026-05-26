#include <algorithm>
#include <array>
#include <charconv>
#include <cctype>
#include <set>
#include <stdexcept>

#include "sqlparser/parser.hpp"

namespace dbms {

Command Parser::parse(const std::string& query) {
    if (query.empty()) {
        return {};
    }

    const auto tokens = tokenize(query);
    if (tokens.empty()) {
        return {};
    }

    Command cmd;
    if (tokens[0] == "CREATE") {
        cmd = parseCreate(tokens);
    } else if (tokens[0] == "DROP") {
        cmd = parseDrop(tokens);
    } else if (tokens[0] == "USE") {
        cmd = parseUse(tokens);
    } else if (tokens[0] == "INSERT") {
        cmd = parseInsert(tokens);
    } else if (tokens[0] == "SELECT") {
        cmd = parseSelect(tokens);
    } else if (tokens[0] == "UPDATE") {
        cmd = parseUpdate(tokens);
    } else if (tokens[0] == "DELETE") {
        cmd = parseDelete(tokens);
    } else {
        throwParseError("Unknown command: " + tokens[0]);
    }

    return cmd;
}

std::vector<std::string> Parser::tokenize(const std::string& input) {
    if (input.empty()) {
        throwParseError("Empty query");
    }

    std::vector<std::string> tokens;
    tokens.reserve(64);

    std::string current;
    current.reserve(64);

    bool in_string = false;

    auto push_current = [&]() {
        if (current.empty()) {
            return;
        }

        if (current.size() > kMaxStringLength) {
            throwParseError("Token too large");
        }

        current = tryNormalizeToken(current);
        tokens.push_back(current);

        if (tokens.size() > kMaxTokens) {
            throwParseError("Too many tokens");
        }

        current.clear();
    };

    for (size_t i = 0; i < input.size(); ++i) {
        const char ch = input[i];

        if (in_string) {
            if (ch == '"') {
                if (current.size() > kMaxStringLength) {
                    throwParseError("String literal too large");
                }

                tokens.push_back("\"" + current + "\"");

                current.clear();
                in_string = false;
                continue;
            }

            if (ch == '\0') {
                throwParseError("Embedded null byte");
            }

            current += ch;
            continue;
        }

        if (ch == '"') {
            push_current();
            in_string = true;
            continue;
        }

        if (i + 1 < input.size()) {
            const std::string two_chars {
                ch,
                input[i + 1]
            };

            if (
                two_chars == "==" ||
                two_chars == "!=" ||
                two_chars == "<=" ||
                two_chars == ">="
            ) {
                push_current();
                tokens.push_back(two_chars);
                ++i;
                continue;
            }
        }

        if (ch == '=' || ch == '<' || ch == '>') {
            push_current();
            tokens.emplace_back(1, ch);
            continue;
        }

        if (ch == '(' || ch == ')' || ch == ',' || ch == ';') {
            push_current();
            tokens.emplace_back(1, ch);
            continue;
        }

        if (IsSpace(ch)) {
            push_current();
            continue;
        }

        if (ch == '\0') {
            throwParseError("Embedded null byte");
        }

        current += ch;
    }

    if (in_string) {
        throwParseError("Unterminated string literal");
    }

    push_current();

    if (tokens.empty()) {
        throwParseError("No tokens");
    }

    return tokens;
}

std::string Parser::normalize(const std::string& token) {
    std::string result;
    result.reserve(token.size());

    for (const char ch : token) {
        if (ch == '(' || ch == ')' || ch == ',' || ch == ';') {
            continue;
        }

        result += ch;
    }

    return result;
}

void Parser::validateIdentifier(const std::string& token) const {
    if (!isValidIdentifier(token)) {
        throwParseError("Invalid identifier: " + token);
    }

    if (isKeyword(token)) {
        throwParseError(
            "Identifier cannot be a keyword: " + token
        );
    }
}

std::string Parser::normalizeKeyword(
    const std::string& token
) const {
    bool has_lower = false;
    bool has_upper = false;

    for (const char ch : token) {
        if (std::islower(static_cast<unsigned char>(ch))) {
            has_lower = true;
        }

        if (std::isupper(static_cast<unsigned char>(ch))) {
            has_upper = true;
        }
    }

    if (has_lower && has_upper) {
        throwParseError("Mixed-case keyword: " + token);
    }

    std::string result;
    result.reserve(token.size());

    for (const char ch : token) {
        result += toUpper(ch);
    }

    return result;
}

std::string Parser::tryNormalizeToken(
    const std::string& token
) const {
    if (isKeyword(token)) {
        return normalizeKeyword(token);
    }

    return token;
}

Command Parser::parseUse(const std::vector<std::string>& tokens) {
    if (tokens.size() != 2) {
        throwParseError("USE requires exactly one database name");
    }

    Command cmd;
    cmd.type = CommandType::kUseDatabase;

    validateIdentifier(tokens[1]);

    cmd.database_name = tokens[1];

    return cmd;
}

Command Parser::parseDrop(
    const std::vector<std::string>& tokens
) {
    if (tokens.size() != 3) {
        throwParseError("DROP syntax error");
    }

    Command cmd;

    if (tokens[1] == "DATABASE") {
        validateIdentifier(tokens[2]);

        cmd.type = CommandType::kDropDatabase;
        cmd.database_name = tokens[2];

        return cmd;
    }

    if (tokens[1] == "TABLE") {
        cmd.type = CommandType::kDropTable;

        parseTableName(tokens[2], cmd.database_name, cmd.table_name);

        return cmd;
    }

    throwParseError("Expected DATABASE or TABLE after DROP");

    return cmd;
}

Command Parser::parseCreate(
    const std::vector<std::string>& tokens
) {
    if (tokens.size() < 3) {
        throwParseError("Incomplete CREATE statement");
    }

    if (tokens[1] == "DATABASE") {
        if (tokens.size() != 3) {
            throwParseError("CREATE DATABASE syntax error");
        }

        validateIdentifier(tokens[2]);

        Command cmd;
        cmd.type = CommandType::kCreateDatabase;
        cmd.database_name = tokens[2];

        return cmd;
    }

    if (tokens[1] == "TABLE") {
        return parseCreateTable(tokens);
    }

    throwParseError("Expected DATABASE or TABLE after CREATE");

    return {};
}

Command Parser::parseCreateTable(
    const std::vector<std::string>& tokens
) {
    if (tokens.size() < 5) {
        throwParseError("Incomplete CREATE TABLE statement");
    }

    Command cmd;
    cmd.type = CommandType::kCreateTable;

    parseTableName(tokens[2], cmd.database_name, cmd.table_name);

    size_t pos = 3;

    if (tokens[pos] != "(") {
        throwParseError("Expected '(' after table name");
    }

    ++pos;

    std::set<std::string> unique_columns;

    while (pos < tokens.size()) {
        if (tokens[pos] == ")") {
            ++pos;
            break;
        }

        if (pos + 1 >= tokens.size()) {
            throwParseError("Incomplete column definition");
        }

        ColumnSchema column;
        column.name = tokens[pos];

        validateIdentifier(column.name);

        if (unique_columns.contains(column.name)) {
            throwParseError("Duplicate column: " + column.name);
        }

        unique_columns.insert(column.name);
        ++pos;

        const std::string& type = tokens[pos];
        if (type == "INT") {
            column.type = ColumnType::kInt;
        } else if (type == "STRING") {
            column.type = ColumnType::kString;
        } else {
            throwParseError("Unknown column type: " + type);
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
                throwParseError(
                    "Unknown column modifier: " +
                    tokens[pos]
                );
            }

            ++pos;
        }

        cmd.columns.push_back(column);

        if (pos >= tokens.size()) {
            throwParseError("Unexpected end of CREATE TABLE");
        }

        if (tokens[pos] == ",") {
            ++pos;
            continue;
        }

        if (tokens[pos] == ")") {
            ++pos;
            break;
        }

        throwParseError("Expected ',' or ')'");
    }

    if (cmd.columns.empty()) {
        throwParseError("Table must contain at least one column");
    }

    if (pos != tokens.size()) {
        throwParseError("Unexpected tokens after CREATE TABLE");
    }

    return cmd;
}

Command Parser::parseInsert(
    const std::vector<std::string>& tokens
) {
    if (tokens.size() < 6) {
        throwParseError("Incomplete INSERT statement");
    }

    if (tokens[1] != "INTO") {
        throwParseError("Expected INTO");
    }

    Command cmd;
    cmd.type = CommandType::kInsert;

    size_t pos = 2;
    parseTableName(tokens[pos], cmd.database_name, cmd.table_name);
    ++pos;

    if (pos >= tokens.size()) {
        throwParseError("Unexpected end after table name");
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
                validateIdentifier(tokens[pos]);

                if (unique_columns.contains(tokens[pos])) {
                    throwParseError(
                        "Duplicate column name: " +
                        tokens[pos]
                    );
                }

                unique_columns.insert(tokens[pos]);
                cmd.column_names.push_back(tokens[pos]);

                expect_identifier = false;

            } else {
                if (tokens[pos] != ",") {
                    throwParseError("Expected ',' between columns");
                }

                expect_identifier = true;
            }

            ++pos;
        }

        if (expect_identifier) {
            throwParseError("Trailing comma in column list");
        }
    }

    if (pos >= tokens.size() || tokens[pos] != "VALUE") {
        throwParseError("Expected VALUE");
    }

    ++pos;

    while (pos < tokens.size()) {
        if (tokens[pos] != "(") {
            throwParseError("Expected '(' in VALUE tuple");
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
                row.push_back(parseValue(tokens[pos]));
                expect_value = false;
            } else {
                if (tokens[pos] != ",") {
                    throwParseError("Expected ',' between values");
                }

                expect_value = true;
            }

            ++pos;
        }

        if (expect_value) {
            throwParseError("Trailing comma in VALUE tuple");
        }

        if (
            !cmd.column_names.empty() &&
            row.size() != cmd.column_names.size()
        ) {
            throwParseError("Column/value count mismatch");
        }

        cmd.values.push_back(std::move(row));

        if (pos >= tokens.size()) {
            break;
        }

        if (tokens[pos] != ",") {
            throwParseError("Expected ',' between VALUE tuples");
        }

        ++pos;
    }

    if (cmd.values.empty()) {
        throwParseError("INSERT requires at least one row");
    }

    return cmd;
}

Command Parser::parseSelect(
    const std::vector<std::string>& tokens
) {
    if (tokens.size() < 4) {
        throwParseError("Incomplete SELECT statement");
    }

    Command cmd;
    cmd.type = CommandType::kSelect;

    size_t pos = 1;

    if (tokens[pos] == "*") {
        SelectColumn col;
        col.name = "*";

        cmd.select_columns.push_back(col);
        ++pos;
    } else {
        bool expect_column = true;

        while (pos < tokens.size()) {
            if (tokens[pos] == "FROM") {
                break;
            }

            if (expect_column) {
                validateIdentifier(tokens[pos]);

                SelectColumn col;
                col.name = tokens[pos];

                ++pos;

                if (
                    pos < tokens.size() &&
                    tokens[pos] == "AS"
                ) {

                    ++pos;

                    if (pos >= tokens.size()) {
                        throwParseError("Expected alias after AS");
                    }

                    if (tokens[pos] == "*") {
                        throwParseError(
                            "'*' allowed only as standalone select expression"
                        );
                    }

                    validateIdentifier(tokens[pos]);
                    col.alias = tokens[pos];
                    ++pos;
                }

                cmd.select_columns.push_back(col);
                expect_column = false;

            } else {
                if (tokens[pos] != ",") {
                    throwParseError("Expected ',' between columns");
                }

                expect_column = true;
                ++pos;
            }
        }

        if (expect_column) {
            throwParseError("Trailing comma in SELECT list");
        }
    }

    if (pos >= tokens.size() || tokens[pos] != "FROM") {
        throwParseError("Expected FROM");
    }

    ++pos;

    if (pos >= tokens.size()) {
        throwParseError("Expected table name after FROM");
    }

    parseTableName(tokens[pos], cmd.database_name, cmd.table_name);
    ++pos;

    if (pos < tokens.size()) {
        if (tokens[pos] != "WHERE") {
            throwParseError("Unexpected token after table name");
        }

        ++pos;

        if (pos >= tokens.size()) {
            throwParseError("WHERE requires conditions");
        }

        cmd.conditions = parseConditions(tokens, pos);
    }

    return cmd;
}

Command Parser::parseUpdate(
    const std::vector<std::string>& tokens
) {
    if (tokens.size() < 6) {
        throwParseError("Incomplete UPDATE statement");
    }

    if (tokens[2] != "SET") {
        throwParseError("Expected SET");
    }

    Command cmd;
    cmd.type = CommandType::kUpdate;

    parseTableName(tokens[1], cmd.database_name, cmd.table_name);

    size_t pos = 3;
    bool expect_assignment = true;
    std::set<std::string> assigned_columns;

    while (pos < tokens.size()) {
        if (tokens[pos] == "WHERE") {
            ++pos;

            if (pos >= tokens.size()) {
                throwParseError("WHERE requires conditions");
            }

            cmd.conditions = parseConditions(tokens, pos);
            return cmd;
        }

        if (expect_assignment) {
            validateIdentifier(tokens[pos]);

            Assignment assignment;
            assignment.column = tokens[pos];

            if (assigned_columns.contains(assignment.column)) {
                throwParseError(
                    "Duplicate assignment: " +
                    assignment.column
                );
            }

            assigned_columns.insert(assignment.column);
            ++pos;

            if (pos >= tokens.size() || tokens[pos] != "=") {
                throwParseError("Expected '='");
            }

            ++pos;

            if (pos >= tokens.size()) {
                throwParseError("Expected value after '='");
            }

            assignment.value = parseValue(tokens[pos]);
            cmd.assignments.push_back(std::move(assignment));
            expect_assignment = false;

        } else {
            if (tokens[pos] != ",") {
                throwParseError("Expected ',' between assignments");
            }

            expect_assignment = true;
        }

        ++pos;
    }

    if (expect_assignment) {
        throwParseError("Trailing comma in SET clause");
    }

    if (cmd.assignments.empty()) {
        throwParseError("UPDATE requires assignments");
    }

    return cmd;
}

Command Parser::parseDelete(
    const std::vector<std::string>& tokens
) {
    if (tokens.size() < 3) {
        throwParseError("Incomplete DELETE statement");
    }

    if (tokens[1] != "FROM") {
        throwParseError("Expected FROM");
    }

    Command cmd;
    cmd.type = CommandType::kDelete;

    parseTableName(tokens[2], cmd.database_name, cmd.table_name);

    size_t pos = 3;

    if (pos < tokens.size()) {
        if (tokens[pos] != "WHERE") {
            throwParseError("Unexpected token after table name");
        }

        ++pos;

        if (pos >= tokens.size()) {
            throwParseError("WHERE requires conditions");
        }

        cmd.conditions = parseConditions(tokens, pos);
    }

    return cmd;
}

std::vector<Condition> Parser::parseConditions(
    const std::vector<std::string>& tokens,
    size_t pos
) {
    std::vector<Condition> conditions;

    bool expect_condition = true;
    while (pos < tokens.size()) {
        if (!expect_condition) {
            if (tokens[pos] == "AND") {
                ++pos;
            }

            expect_condition = true;
            continue;
        }

        if (pos >= tokens.size()) {
            throwParseError("Incomplete condition");
        }

        Condition condition;
        condition.lhs = parseOperand(tokens[pos]);

        ++pos;

        if (pos >= tokens.size()) {
            throwParseError("Missing operator in condition");
        }

        const std::string op = tokens[pos];

        if (!IsOperatorToken(op)) {
            throwParseError("Invalid operator: " + op);
        }

        condition.operator_type = op;
        ++pos;

        if (op == "BETWEEN") {
            if (pos >= tokens.size()) {
                throwParseError("BETWEEN requires lower bound");
            }

            condition.rhs = parseOperand(tokens[pos]);
            ++pos;

            if (pos >= tokens.size() || tokens[pos] != "AND") {
                throwParseError("BETWEEN requires AND");
            }

            ++pos;

            if (pos >= tokens.size()) {
                throwParseError("BETWEEN requires upper bound");
            }

            condition.range_end = parseOperand(tokens[pos]);
            ++pos;

        } else {
            if (pos >= tokens.size()) {
                throwParseError("Operator requires RHS operand");
            }

            condition.rhs = parseOperand(tokens[pos]);
            ++pos;
        }

        conditions.push_back(condition);
        expect_condition = false;
    }

    if (conditions.empty()) {
        throwParseError("Empty WHERE clause");
    }

    return conditions;
}

Value Parser::parseValue(const std::string& token) {
    if (token == "NULL") {
        return Value();
    }

    if (
        token.size() >= 2 &&
        token.front() == '"' &&
        token.back() == '"'
    ) {
        return Value(token.substr(1, token.size() - 2));
    }

    bool is_number = true;

    for (size_t i = 0; i < token.size(); ++i) {
        const char ch = token[i];

        if (i == 0 && ch == '-') {
            continue;
        }

        if (!IsDigit(ch)) {
            is_number = false;
            break;
        }
    }

    if (is_number && !token.empty() && token != "-") {
        int value = 0;

        const auto* begin = token.data();
        const auto* end = token.data() + token.size();

        const auto result = std::from_chars(
            begin,
            end,
            value
        );

        if (result.ec == std::errc::result_out_of_range) {
            throwParseError("Integer overflow: " + token);
        }

        if (result.ec != std::errc() || result.ptr != end) {
            throwParseError("Invalid integer: " + token);
        }

        return Value(value);
    }

    validateIdentifier(token);

    return Value(token);
}

Operand Parser::parseOperand(const std::string& token) {
    Operand op;

    if (token == "NULL") {
        op.value = Value();
        return op;
    }

    if (
        token.size() >= 2 &&
        token.front() == '"' &&
        token.back() == '"'
    ) {
        op.value = Value(token.substr(1, token.size() - 2));
        return op;
    }

    bool is_number = true;

    for (size_t i = 0; i < token.size(); ++i) {
        const char ch = token[i];

        if (i == 0 && ch == '-') {
            continue;
        }

        if (!IsDigit(ch)) {
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

        if (
            result.ec != std::errc() ||
            result.ptr != token.data() + token.size()
        ) {
            throwParseError("Invalid integer: " + token);
        }

        op.value = Value(value);
        return op;
    }

    validateIdentifier(token);

    op.is_column = true;
    op.column = token;

    return op;
}

void Parser::parseTableName(
    const std::string& full_name,
    std::string& database_name,
    std::string& table_name
) {
    if (full_name.empty()) {
        throwParseError("Empty table name");
    }

    const size_t first_dot = full_name.find('.');

    if (first_dot == std::string::npos) {
        validateIdentifier(full_name);

        database_name.clear();
        table_name = full_name;

        return;
    }

    const size_t second_dot = full_name.find('.', first_dot + 1);

    if (second_dot != std::string::npos) {
        throwParseError("Too many dots in table name");
    }

    if (first_dot == 0 || first_dot == full_name.size() - 1) {
        throwParseError("Malformed qualified table name");
    }

    database_name = full_name.substr(0, first_dot);
    table_name = full_name.substr(first_dot + 1);

    validateIdentifier(database_name);
    validateIdentifier(table_name);
}

bool Parser::isValidIdentifier(const std::string& token) const {
    if (token.empty()) {
        return false;
    }

    if (token.size() > kMaxIdentifierLength) {
        return false;
    }

    if (IsDigit(token.front())) {
        return false;
    }

    for (const char ch : token) {
        if (IsAlnum(ch) || ch == '_') {
            continue;
        }

        return false;
    }

    return true;
}

bool Parser::isKeyword(const std::string& token) const {
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
        upper += toUpper(ch);
    }

    return std::ranges::find(keywords, upper) != keywords.end();
}

bool Parser::IsSpace(const char ch) const {
    return std::isspace(static_cast<unsigned char>(ch)) != 0;
}

bool Parser::IsDigit(const char ch) const {
    return std::isdigit(static_cast<unsigned char>(ch)) != 0;
}

bool Parser::IsAlpha(const char ch) const {
    return std::isalpha(static_cast<unsigned char>(ch)) != 0;
}

bool Parser::IsAlnum(const char ch) const {
    return std::isalnum(static_cast<unsigned char>(ch)) != 0;
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

char Parser::toUpper(const char ch) const {
    return static_cast<char>(
        std::toupper(static_cast<unsigned char>(ch))
    );
}

void Parser::throwParseError(const std::string& message) const {
    throw std::runtime_error("Parser error: " + message);
}

} // namespace dbms

