#include "../parser.h"

#include "parser_support.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <unordered_set>
#include <utility>

std::shared_ptr<Statement> Parser::parseStatement() {
    Token command = consume(TokenKind::Identifier, "Expected statement.");
    const int line = command.line;
    auto stamp = [line](std::shared_ptr<Statement> statement) {
        statement->line = line;
        return statement;
    };

    if (command.text == "switch_view") {
        --position_;
        return stamp(parseSwitchViewStatement());
    }

    if (command.text == "add_view") {
        --position_;
        return stamp(parseAddViewStatement());
    }

    if (command.text == "if") {
        --position_;
        return stamp(parseIfStatement());
    }

    if (command.text == "while") {
        --position_;
        return stamp(parseWhileStatement());
    }

    if (command.text == "for") {
        --position_;
        return stamp(parseForStatement());
    }

    if (command.text == "print" || command.text == "println") {
        auto statement = std::make_shared<Statement>();
        statement->value = parsePrintStatement(command.text == "println");
        return stamp(statement);
    }

    if (current().kind == TokenKind::Dot &&
        peek(1).kind == TokenKind::Identifier &&
        peek(2).kind == TokenKind::OpenParen) {
        consume(TokenKind::Dot, "Expected '.' in method call.");
        const Token method = consume(
            TokenKind::Identifier,
            "Expected method name."
        );
        if (method.text == "add" || method.text == "remove") {
            return stamp(parseWindowObjectStatement(command.text, method.text));
        }
        if (method.text != "update") {
            throw std::runtime_error(
                "Only window.update() can be called from a statement."
            );
        }
        return stamp(parseWindowUpdateStatement(command.text));
    }

    if (command.text == "int" || command.text == "short" ||
        command.text == "long" || command.text == "float" ||
        command.text == "double" || command.text == "bool" ||
        command.text == "char" || command.text == "String") {
        auto statement = std::make_shared<Statement>();
        statement->value = parseVariableDeclaration(command.text);
        return stamp(statement);
    }

    if (current().kind == TokenKind::Increment ||
        current().kind == TokenKind::Decrement) {
        --position_;
        return stamp(parseIncrementStatement());
    }

    if (current().kind == TokenKind::Equals) {
        --position_;
        return stamp(parseVariableAssignment());
    }

    if (current().kind == TokenKind::Dot) {
        if (peek(2).kind == TokenKind::Increment ||
            peek(2).kind == TokenKind::Decrement) {
            --position_;
            return stamp(parseIncrementStatement());
        }
        --position_;
        auto statement = std::make_shared<Statement>();
        statement->value = parsePropertyAssignment();
        return stamp(statement);
    }

    throw std::runtime_error("Unknown statement.");
}

std::shared_ptr<Statement> Parser::parseSwitchViewStatement() {
    consumeIdentifier("switch_view");
    consume(TokenKind::OpenParen, "Expected '(' after switch_view.");

    const std::string windowId = consume(
        TokenKind::Identifier,
        "Expected window object name."
    ).text;

    consume(TokenKind::CloseParen, "Expected ')' after switch_view.");
    consume(TokenKind::Semicolon, "Expected ';' after switch_view.");

    auto statement = std::make_shared<Statement>();
    statement->value = SwitchViewStatement{windowId};
    return statement;
}

std::shared_ptr<Statement> Parser::parseAddViewStatement() {
    consumeIdentifier("add_view");
    consume(TokenKind::OpenParen, "Expected '(' after add_view.");

    const std::string windowId = consume(
        TokenKind::Identifier,
        "Expected window object name."
    ).text;

    consume(TokenKind::CloseParen, "Expected ')' after add_view.");
    consume(TokenKind::Semicolon, "Expected ';' after add_view.");

    auto statement = std::make_shared<Statement>();
    statement->value = AddViewStatement{windowId};
    return statement;
}

std::vector<std::shared_ptr<Statement>> Parser::parseStatementBlock(
    const char* owner
) {
    consume(TokenKind::OpenBrace, owner);
    std::vector<std::shared_ptr<Statement>> statements;

    while (!match(TokenKind::CloseBrace)) {
        statements.push_back(parseStatement());
    }

    return statements;
}

ValueExpression Parser::parseValueExpression() {
    return parseAdditiveExpression();
}

ValueExpression Parser::parseAdditiveExpression() {
    ValueExpression expression = parseMultiplicativeExpression();

    while (current().kind == TokenKind::Plus ||
           current().kind == TokenKind::Minus) {
        auto arithmetic = std::make_shared<ArithmeticExpression>();
        arithmetic->left = expression;
        arithmetic->operation = current().kind == TokenKind::Plus ? '+' : '-';
        ++position_;
        arithmetic->right = parseMultiplicativeExpression();
        expression = {};
        expression.arithmetic = std::move(arithmetic);
    }

    return expression;
}

ValueExpression Parser::parseMultiplicativeExpression() {
    ValueExpression expression = parsePrimaryExpression();

    while (current().kind == TokenKind::Star ||
           current().kind == TokenKind::Slash ||
           current().kind == TokenKind::Percent) {
        auto arithmetic = std::make_shared<ArithmeticExpression>();
        arithmetic->left = expression;
        arithmetic->operation = current().kind == TokenKind::Star
            ? '*'
            : current().kind == TokenKind::Slash ? '/' : '%';
        ++position_;
        arithmetic->right = parsePrimaryExpression();
        expression = {};
        expression.arithmetic = std::move(arithmetic);
    }

    return expression;
}

ValueExpression Parser::parsePrimaryExpression() {
    ValueExpression expression;

    if (match(TokenKind::OpenParen)) {
        expression = parseValueExpression();
        consume(TokenKind::CloseParen, "Expected ')' after expression.");
        return expression;
    }

    if (current().kind == TokenKind::String) {
        expression.literal = consume(TokenKind::String, "Expected value.").text;
    } else if (current().kind == TokenKind::Character) {
        expression.literal = consume(TokenKind::Character, "Expected value.").text[0];
    } else if (current().kind == TokenKind::Number) {
        std::string value = consume(TokenKind::Number, "Expected value.").text;
        if (value.find('.') == std::string::npos) {
            try {
                expression.literal = std::stoi(value);
            } catch (const std::out_of_range&) {
                try {
                    expression.literal = std::stoll(value);
                } catch (...) {
                    throw std::runtime_error("Integer literal is too large.");
                }
            }
        } else {
            const double number = std::stod(value);
            if (std::isfinite(number) &&
                std::abs(number) <= std::numeric_limits<float>::max()) {
                expression.literal = static_cast<float>(number);
            } else {
                expression.literal = number;
            }
        }

    } else {
        std::string name = consume(TokenKind::Identifier, "Expected value.").text;

        if (match(TokenKind::Dot)) {
            name += "." + consume(
                TokenKind::Identifier,
                "Expected property after '.'."
            ).text;
        }

        if (name == "true") expression.literal = true;
        else if (name == "false") expression.literal = false;
        else if (isFontConstant(name)) expression.literal = name;
        else expression.variable = name;
    }

    return expression;
}

PrintStatement Parser::parsePrintStatement(bool newline) {
    PrintStatement statement;
    statement.newline = newline;
    consume(TokenKind::OpenParen, "Expected '(' after print command.");

    if (match(TokenKind::CloseParen)) {
        if (!newline) {
            throw std::runtime_error("print() requires one argument; use println() for a blank line.");
        }
        consume(TokenKind::Semicolon, "Expected ';' after println().");
        return statement;
    }

    if (current().kind == TokenKind::Identifier && current().text == "f" &&
        peek(1).kind == TokenKind::String) {
        consume(TokenKind::Identifier, "Expected formatted string prefix.");
        statement.formatted = true;
        statement.parts = parseFormattedParts(
            consume(TokenKind::String, "Expected formatted string.").text
        );
    } else {
        statement.expression = parseValueExpression();
    }

    consume(TokenKind::CloseParen, "Expected ')' after print argument.");
    consume(TokenKind::Semicolon, "Expected ';' after print statement.");
    return statement;
}

std::vector<FormattedPart> Parser::parseFormattedParts(const std::string& text) {
    std::vector<FormattedPart> parts;
    std::size_t start = 0;
    while (start < text.size()) {
        const std::size_t open = text.find('{', start);
        if (open == std::string::npos) {
            if (start < text.size()) parts.push_back({text.substr(start)});
            break;
        }
        if (open > start) parts.push_back({text.substr(start, open - start)});
        const std::size_t close = text.find('}', open + 1);
        if (close == std::string::npos) {
            throw std::runtime_error("Unclosed formatted string expression.");
        }
        Parser inner(Lexer(text.substr(open + 1, close - open - 1)).tokenize());
        FormattedPart part;
        part.value = inner.parseValueExpression();
        if (inner.current().kind != TokenKind::End) {
            throw std::runtime_error("Unexpected content in formatted string expression.");
        }
        parts.push_back(std::move(part));
        start = close + 1;
    }
    if (text.empty()) parts.push_back({std::string{}});
    return parts;
}

ComparisonOperator Parser::parseComparisonOperator() {
    if (match(TokenKind::EqualEqual)) return ComparisonOperator::Equal;
    if (match(TokenKind::NotEqual)) return ComparisonOperator::NotEqual;
    if (match(TokenKind::Less)) return ComparisonOperator::Less;
    if (match(TokenKind::LessEqual)) return ComparisonOperator::LessEqual;
    if (match(TokenKind::Greater)) return ComparisonOperator::Greater;
    if (match(TokenKind::GreaterEqual)) return ComparisonOperator::GreaterEqual;
    throw std::runtime_error("Expected comparison operator.");
}

Condition Parser::parseCondition(bool parenthesized) {
    if (parenthesized) {
        consume(TokenKind::OpenParen, "Expected '(' before condition.");
    }
    Condition condition = parseConditionOr();
    if (parenthesized) {
        consume(TokenKind::CloseParen, "Expected ')' after condition.");
    }
    return condition;
}

Condition Parser::parseConditionOr() {
    Condition condition = parseConditionAnd();
    while (match(TokenKind::OrOr)) {
        auto combined = std::make_shared<Condition>();
        combined->logical = Condition::LogicalOperator::Or;
        combined->leftCondition = std::make_shared<Condition>(std::move(condition));
        combined->rightCondition = std::make_shared<Condition>(parseConditionAnd());
        condition = std::move(*combined);
    }
    return condition;
}

Condition Parser::parseConditionAnd() {
    Condition condition = parseConditionPrimary();
    while (match(TokenKind::AndAnd)) {
        auto combined = std::make_shared<Condition>();
        combined->logical = Condition::LogicalOperator::And;
        combined->leftCondition = std::make_shared<Condition>(std::move(condition));
        combined->rightCondition = std::make_shared<Condition>(parseConditionPrimary());
        condition = std::move(*combined);
    }
    return condition;
}

Condition Parser::parseConditionPrimary() {
    if (match(TokenKind::OpenParen)) {
        Condition condition = parseConditionOr();
        consume(TokenKind::CloseParen, "Expected ')' after condition.");
        return condition;
    }

    Condition condition;

    if (match(TokenKind::Bang)) {
        condition.left = parseValueExpression();
        condition.hasComparison = false;
        condition.negated = true;
        return condition;
    }

    condition.left = parseValueExpression();

    if (current().kind == TokenKind::CloseParen ||
        current().kind == TokenKind::AndAnd || current().kind == TokenKind::OrOr) {
        condition.hasComparison = false;
        return condition;
    }

    condition.operation = parseComparisonOperator();
    condition.right = parseValueExpression();
    return condition;
}

std::shared_ptr<Statement> Parser::parseIfStatement() {
    consumeIdentifier("if");
    auto statement = std::make_shared<Statement>();
    auto node = std::make_shared<IfStatement>();
    node->condition = parseCondition();
    node->body = parseStatementBlock("Expected '{' after if condition.");

    if (current().kind == TokenKind::Identifier && current().text == "else") {
        consumeIdentifier("else");

        if (current().kind == TokenKind::Identifier && current().text == "if") {
            node->elseBody.push_back(parseIfStatement());
        } else {
            node->elseBody = parseStatementBlock("Expected '{' after else.");
        }
    }

    statement->value = std::move(node);
    return statement;
}

std::shared_ptr<Statement> Parser::parseWhileStatement() {
    consumeIdentifier("while");
    auto statement = std::make_shared<Statement>();
    auto node = std::make_shared<WhileStatement>();
    node->condition = parseCondition();
    node->body = parseStatementBlock("Expected '{' after while condition.");
    statement->value = std::move(node);
    return statement;
}

std::shared_ptr<Statement> Parser::parseForStatement() {
    consumeIdentifier("for");
    consume(TokenKind::OpenParen, "Expected '(' after for.");

    Token type = consume(TokenKind::Identifier, "Expected for initializer type.");
    if (type.text != "int" && type.text != "short" && type.text != "long" &&
        type.text != "float" && type.text != "double") {
        throw std::runtime_error("For initializer must use a numeric type.");
    }

    auto node = std::make_shared<ForStatement>();
    node->initializer = parseVariableDeclaration(type.text);
    node->condition = parseCondition(false);
    consume(TokenKind::Semicolon, "Expected ';' after for condition.");
    node->incrementVariable = consume(
        TokenKind::Identifier,
        "Expected for increment variable."
    ).text;
    consume(TokenKind::Increment, "Expected '++' in for loop.");
    consume(TokenKind::CloseParen, "Expected ')' after for clauses.");
    node->body = parseStatementBlock("Expected '{' after for clauses.");

    auto statement = std::make_shared<Statement>();
    statement->value = std::move(node);
    return statement;
}

std::shared_ptr<Statement> Parser::parseIncrementStatement() {
    std::string target = consume(
        TokenKind::Identifier,
        "Expected variable or widget property before '++'."
    ).text;

    std::string property;
    if (match(TokenKind::Dot)) {
        property = consume(
            TokenKind::Identifier,
            "Expected numeric property before '++'."
        ).text;
        if (property != "fontSize" && property != "radius" && property != "angle" &&
            property != "x" && property != "y") {
            throw std::runtime_error(
                "Only x, y, fontSize, radius, and angle can be incremented on widgets."
            );
        }
    }

    const bool decrement = match(TokenKind::Decrement);
    if (!decrement) {
        consume(TokenKind::Increment, "Expected '++' or '--'.");
    }
    consume(TokenKind::Semicolon, "Expected ';' after increment.");

    auto statement = std::make_shared<Statement>();
    statement->value = IncrementStatement{target, property, decrement};
    return statement;
}

std::shared_ptr<Statement> Parser::parseVariableAssignment() {
    VariableAssignment assignment;
    assignment.name = consume(
        TokenKind::Identifier,
        "Expected variable name before '='."
    ).text;
    consume(TokenKind::Equals, "Expected '=' after variable name.");
    assignment.value = parseValueExpression();
    consume(TokenKind::Semicolon, "Expected ';' after assignment.");

    auto statement = std::make_shared<Statement>();
    statement->value = std::move(assignment);
    return statement;
}

std::shared_ptr<Statement> Parser::parseTriggerStatement() {
    consumeIdentifier("trigger");
    auto node = parseTriggerBody();

    auto statement = std::make_shared<Statement>();
    statement->value = node;
    return statement;
}

std::shared_ptr<TriggerStatement> Parser::parseTriggerBody() {
    auto node = std::make_shared<TriggerStatement>();

    consume(TokenKind::OpenParen, "Expected '(' after trigger.");

    if (current().kind == TokenKind::Identifier && current().text == "mouseMove") {
        consume(TokenKind::Identifier, "Expected trigger event.");
        node->mouseMove = true;
    } else if (current().kind == TokenKind::Identifier &&
               current().text == "closeTrigger") {
        consume(TokenKind::Identifier, "Expected trigger event.");
        node->closeTrigger = true;
    } else if (current().kind == TokenKind::Identifier &&
               (current().text == "onkey" || current().text == "onkeyci")) {
        const bool insensitive = current().text == "onkeyci";
        ++position_;
        consume(TokenKind::OpenParen, "Expected '(' after key trigger.");
        const std::string key = consumeString("Expected key name in key trigger.");
        consume(TokenKind::CloseParen, "Expected ')' after key name.");
        static const std::vector<std::string> special = {
            "space", "esc", "enter", "shift", "alt", "up", "down", "left", "right", "tab"
        };
        const bool singleLetter = key.size() == 1 &&
            std::isalpha(static_cast<unsigned char>(key[0]));
        if (!singleLetter &&
            std::find(special.begin(), special.end(), key) == special.end()) {
            throw std::runtime_error("Key triggers accept one letter or a named special key.");
        }
        node->key = key;
        node->keyCaseInsensitive = insensitive;
    } else {
        node->condition = parseCondition(false);
    }

    consume(TokenKind::CloseParen, "Expected ')' after trigger condition.");
    node->body = parseStatementBlock("Expected '{' after trigger condition.");

    return node;
}

std::shared_ptr<Statement> Parser::parseWindowUpdateStatement(
    const std::string& windowId
) {
    consume(TokenKind::OpenParen, "Expected '(' after update.");
    consume(TokenKind::CloseParen, "Expected ')' after update.");
    consume(TokenKind::Semicolon, "Expected ';' after update call.");

    auto statement = std::make_shared<Statement>();
    statement->value = WindowUpdateStatement{windowId};
    return statement;
}

std::shared_ptr<Statement> Parser::parseWindowObjectStatement(
    const std::string& windowId,
    const std::string& method
) {
    consume(TokenKind::OpenParen, "Expected '(' after window method.");
    const std::string objectId = consume(
        TokenKind::Identifier,
        "Expected object name in window method."
    ).text;
    consume(TokenKind::CloseParen, "Expected ')' after object name.");
    consume(TokenKind::Semicolon, "Expected ';' after window method.");

    auto statement = std::make_shared<Statement>();
    statement->value = WindowObjectStatement{
        windowId,
        objectId,
        method == "add"
    };
    return statement;
}
