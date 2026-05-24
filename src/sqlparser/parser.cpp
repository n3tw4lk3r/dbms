#include <array>
#include <stdexcept>

#include "sqlparser/parser.hpp"

namespace dbms {

std::vector<std::string> Parser::tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::string current;
    bool in_string = false;

    for (size_t i = 0; i < input.size(); ++i) {
        char ch = input[i];

        if (ch == '"') {
            if (in_string) {
                tokens.push_back("\"" + current + "\"");
                current.clear();
                in_string = false;
            } else {
                in_string = true;
            }
            continue;
        }

        if (in_string) {
            current += ch;
            continue;
        }

        if (!in_string) {
            if (i + 1 < input.size()) {
                std::string two {ch, input[i + 1]};
                if (two == "==" || two == "!=" || two == "<=" || two == ">=") {
                    if (!current.empty()) {
                        current = tryNormalizeToken(current);
                        tokens.push_back(current);
                        current.clear();
                    }

                    tokens.push_back(two);
                    ++i;
                    continue;
                }
            }

        }

        if (std::isspace(ch)) {
            if (!current.empty()) {
                current = tryNormalizeToken(current);
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }

        if (ch == '(' || ch == ')' || ch == ',' || ch == ';') {
            if (!current.empty()) {
                current = tryNormalizeToken(current);
                tokens.push_back(current);
                current.clear();
            }

            tokens.emplace_back(1, ch);
            continue;
        }

        current += ch;
    }

    if (!current.empty()) {
        current = tryNormalizeToken(current);
        tokens.push_back(current);
    }

    return tokens;
}

std::string Parser::normalize(const std::string& token) {
    std::string result;

    for (char ch : token) {
        if (ch == '(' || ch == ')' || ch == ',' || ch == ';') {
            continue;
        }

        result += ch;
    }

    return result;
}

bool Parser::isValidIdentifier(
    const std::string& token
) const {
    if (token.empty()) {
        return false;
    }

    if (std::isdigit(static_cast<unsigned char>(token[0]))) {
        return false;
    }

    for (char ch : token) {
        if (std::isalnum(
            static_cast<unsigned char>(ch)) ||
            ch == '_'
        ) {
            continue;
        }

        return false;
    }

    return true;
}

void Parser::validateIdentifier(
    const std::string& token
) const {
    if (!isValidIdentifier(token)) {
        throw std::runtime_error(
            "Invalid identifier: " +
            token
        );
    }
}

bool Parser::isKeyword(const std::string& token) const {
    static constexpr std::array<std::string, 23> keywords = {
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
    for (char ch : token) {
        upper += std::toupper(static_cast<unsigned char>(ch));
    }

    for (const auto& keyword : keywords) {
        if (upper == keyword) {
            return true;
        }
    }

    return false;
}

std::string Parser::normalizeKeyword(
    const std::string& token
) const {
    bool has_lower = false;
    bool has_upper = false;

    for (char ch : token) {
        if (std::islower(static_cast<unsigned char>(ch))) {
            has_lower = true;
        }

        if (std::isupper(static_cast<unsigned char>(ch))) {
            has_upper = true;
        }
    }

    if (has_lower && has_upper) {
        throw std::runtime_error(
            "Mixed-case keywords are not allowed: " +
            token
        );
    }

    std::string result;
    for (char ch : token) {
        result += std::toupper(
            static_cast<unsigned char>(ch)
        );
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

Command Parser::parse(const std::string& query) {
    auto tokens = tokenize(query);

    if (tokens.empty()) {
        return {};
    }

    if (tokens[0] == "CREATE") {
        return parseCreate(tokens);
    }

    if (tokens[0] == "DROP") {
        return parseDrop(tokens);
    }

    if (tokens[0] == "USE") {
        return parseUse(tokens);
    }

    if (tokens[0] == "INSERT") {
        return parseInsert(tokens);
    }

    if (tokens[0] == "SELECT") {
        return parseSelect(tokens);
    }

    if (tokens[0] == "UPDATE") {
        return parseUpdate(tokens);
    }

    if (tokens[0] == "DELETE") {
        return parseDelete(tokens);
    }

    return {};
}

Command Parser::parseUse(const std::vector<std::string>& tokens) {
    Command cmd;
    cmd.type = CommandType::kUseDatabase;

    if (tokens.size() >= 2) {
        cmd.database_name = tokens[1];
    }

    return cmd;
}

Command Parser::parseDrop(const std::vector<std::string>& tokens) {
    Command cmd;

    if (tokens.size() < 3) {
        return cmd;
    }

    if (tokens[1] == "DATABASE") {
        cmd.type = CommandType::kDropDatabase;
        cmd.database_name = tokens[2];
    }

    if (tokens[1] == "TABLE") {
        cmd.type = CommandType::kDropTable;
        parseTableName(tokens[2], cmd.database_name, cmd.table_name);
    }

    return cmd;
}

Command Parser::parseInsert(const std::vector<std::string>& tokens) {
    Command cmd;
    cmd.type = CommandType::kInsert;

    size_t pos = 2;
    parseTableName(tokens[pos], cmd.database_name, cmd.table_name);
    ++pos;

    if (tokens[pos] == "(") {
        ++pos;
        while (tokens[pos] != ")") {
            if (tokens[pos] != ",") {
                cmd.column_names.push_back(tokens[pos]);
            }
            ++pos;
        }
        ++pos;
    }

    if (tokens[pos] == "VALUE") {
        ++pos;
    }

    while (pos < tokens.size()) {
        if (tokens[pos] == "(") {
            ++pos;
            std::vector<Value> row;

            while (tokens[pos] != ")") {
                if (tokens[pos] != ",") {
                    row.push_back(parseValue(tokens[pos]));
                }
                ++pos;
            }

            cmd.values.push_back(row);
        }

        ++pos;
    }

    return cmd;
}

Command Parser::parseSelect(const std::vector<std::string>& tokens) {
    Command cmd;
    cmd.type = CommandType::kSelect;

    size_t pos = 1;
    if (tokens[pos] == "*") {
        SelectColumn col;
        col.name = "*";
        cmd.select_columns.push_back(col);
        pos += 2; // skip * and FROM
    } else {
        while (pos < tokens.size() && tokens[pos] != "FROM") {
            if (tokens[pos] != ",") {
                SelectColumn col;
                col.name = tokens[pos];

                if (pos + 2 < tokens.size() && tokens[pos + 1] == "AS") {
                    col.alias = tokens[pos + 2];
                    pos += 2;
                }

                cmd.select_columns.push_back(col);
            }
            ++pos;
        }
        ++pos;
    }

    parseTableName(tokens[pos], cmd.database_name, cmd.table_name);
    ++pos;

    if (pos < tokens.size() && tokens[pos] == "WHERE") {
        ++pos;
        cmd.conditions = parseConditions(tokens, pos);
    }

    return cmd;
}

Command Parser::parseUpdate(const std::vector<std::string>& tokens) {
    Command cmd;
    cmd.type = CommandType::kUpdate;
    parseTableName(tokens[1], cmd.database_name, cmd.table_name);

    size_t pos = 3;
    while (pos < tokens.size()) {
        if (tokens[pos] == "WHERE") {
            ++pos;
            cmd.conditions = parseConditions(tokens, pos);
            break;
        }

        if (tokens[pos] == ",") {
            ++pos;
            continue;
        }

        Assignment a;

        a.column = tokens[pos];
        pos += 2; // skip column and '='
        a.value = parseValue(tokens[pos]);

        cmd.assignments.push_back(a);

        ++pos;
    }

    return cmd;
}

Command Parser::parseDelete(const std::vector<std::string>& tokens) {
    Command cmd;
    cmd.type = CommandType::kDelete;

    if (tokens.size() < 3) {
        return cmd;
    }

    if (tokens[1] != "FROM") {
        return cmd;
    }

    parseTableName(tokens[2], cmd.database_name, cmd.table_name);
    size_t pos = 3;
    if (pos < tokens.size() && tokens[pos] == "WHERE") {
        ++pos;
        cmd.conditions = parseConditions(tokens, pos);
    }

    return cmd;
}

Command Parser::parseCreate(const std::vector<std::string>& tokens) {
    Command cmd;

    if (tokens.size() < 3) {
        return cmd;
    }

    if (tokens[1] == "DATABASE") {
        validateIdentifier(tokens[2]);

        cmd.type = CommandType::kCreateDatabase;
        cmd.database_name = tokens[2];

        return cmd;
    }

    if (tokens[1] == "TABLE") {
        return parseCreateTable(tokens);
    }

    return cmd;
}

Command Parser::parseCreateTable(const std::vector<std::string>& tokens) {
    Command cmd;
    cmd.type = CommandType::kCreateTable;

    if (tokens.size() < 4) {
        return cmd;
    }

    parseTableName(tokens[2], cmd.database_name, cmd.table_name);
    size_t pos = 3;

    if (tokens[pos] != "(") {
        return cmd;
    }

    ++pos;

    while (pos < tokens.size()) {
        if (tokens[pos] == ")") {
            break;
        }

        ColumnSchema column;
        column.name = tokens[pos];
        ++pos;
        std::string type = tokens[pos];
        ++pos;

        if (type == "INT") {
            column.type = ColumnType::kInt;
        } else if (type == "STRING") {
            column.type = ColumnType::kString;
        }

        while (pos < tokens.size()) {
            if (tokens[pos] == "," || tokens[pos] == ")") {
                break;
            }

            if (tokens[pos] == "NOT_NULL") {
                column.not_null = true;
            }

            if (tokens[pos] == "INDEXED") {
                column.indexed = true;
                column.not_null = true;
            }

            ++pos;
        }

        cmd.columns.push_back(column);

        if (tokens[pos] == ",") {
            ++pos;
        }
    }

    return cmd;
}

Value Parser::parseValue(const std::string& token) {
    if (token == "NULL") {
        return Value();
    }

    if (!token.empty() && (std::isdigit(token[0]) ||
        (token[0] == '-' && token.size() > 1))
    ) {
        return Value(std::stoi(token));
    }

    if (token.size() >= 2 && token.front() == '"' && token.back() == '"') {
        return Value(token.substr(1, token.size() - 2));
    }

    return Value(token);
}

std::vector<Condition> Parser::parseConditions(
    const std::vector<std::string>& tokens,
    size_t pos
) {
    std::vector<Condition> conditions;
    while (pos < tokens.size()) {
        Condition condition;
        condition.lhs = parseOperand(tokens[pos]);
        ++pos;

        std::string op = tokens[pos];
        ++pos;

        if (op == "BETWEEN") {
            condition.operator_type = "BETWEEN";
            condition.rhs = parseOperand(tokens[pos]);
            ++pos;

            if (tokens[pos] != "AND") {
                throw std::runtime_error("Expected AND");
            }

            ++pos;

            condition.range_end = parseOperand(tokens[pos]);
            ++pos;

        } else if (op == "LIKE") {
            condition.operator_type = "LIKE";
            condition.rhs = parseOperand(tokens[pos]);
            ++pos;

        } else {
            condition.operator_type = op;
            condition.rhs = parseOperand(tokens[pos]);
            ++pos;
        }

        conditions.push_back(condition);
    }

    return conditions;
}

Operand Parser::parseOperand(const std::string& token) {
    Operand op;

    if (token.size() >= 2 && token.front() == '"' && token.back() == '"') {
        op.value = Value(token.substr(1, token.size() - 2));
        return op;
    }

    if (token == "NULL") {
        op.value = Value();
        return op;
    }

    if (!token.empty() && (std::isdigit(token[0]) ||
         (token[0] == '-' && token.size() > 1))
    ) {
        op.value = Value(std::stoi(token));
        return op;
    }

    op.is_column = true;
    op.column = token;

    return op;
}

void Parser::parseTableName(
    const std::string& fullName,
    std::string& database_name,
    std::string& table_name
) {
    size_t dot_pos = fullName.find('.');

    if (dot_pos != std::string::npos) {
        database_name = fullName.substr(0, dot_pos);
        table_name = fullName.substr(dot_pos + 1);

        validateIdentifier(database_name);
        validateIdentifier(table_name);

    } else {
        table_name = fullName;

        validateIdentifier(table_name);

        database_name.clear();
    }
}

} // namespace dbms
