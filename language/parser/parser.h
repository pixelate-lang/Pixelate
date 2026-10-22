#pragma once

#include "../ast.h"
#include "../lexer.h"

#include <vector>

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    ProgramNode parse();
    int currentLine() const;

private:
    const Token& current() const;
    const Token& peek(std::size_t offset) const;
    bool match(TokenKind kind);
    Token consume(TokenKind kind, const char* message);
    void consumeFieldSeparator(const char* message);
    void consumeIdentifier(const char* value);

    std::string consumeString(const char* message);
    int consumeNumber(const char* message);

    ElementNode parseElement(ElementType type);
    WindowNode parseWindowBlock();
    void parseElementIntoProgram(ProgramNode& program);
    void parseObjectMethod(ProgramNode& program);
    void parseTriggerDeclaration(ProgramNode& program);
    VariableDeclaration parseVariableDeclaration(
        const std::string& typeName
    );
    std::shared_ptr<Statement> parseStatement();
    std::vector<std::shared_ptr<Statement>> parseStatementBlock(
        const char* owner
    );
    ValueExpression parseValueExpression();
    ValueExpression parseAdditiveExpression();
    ValueExpression parseMultiplicativeExpression();
    ValueExpression parsePrimaryExpression();
    PrintStatement parsePrintStatement(bool newline);
    std::vector<FormattedPart> parseFormattedParts(const std::string& text);
    Condition parseCondition(bool parenthesized = true);
    Condition parseConditionOr();
    Condition parseConditionAnd();
    Condition parseConditionPrimary();
    ComparisonOperator parseComparisonOperator();
    std::shared_ptr<Statement> parseIfStatement();
    std::shared_ptr<Statement> parseWhileStatement();
    std::shared_ptr<Statement> parseForStatement();
    std::shared_ptr<Statement> parseIncrementStatement();
    std::shared_ptr<Statement> parseVariableAssignment();
    std::shared_ptr<Statement> parseSwitchViewStatement();
    std::shared_ptr<Statement> parseAddViewStatement();
    std::shared_ptr<Statement> parseTriggerStatement();
    std::shared_ptr<TriggerStatement> parseTriggerBody();
    std::shared_ptr<Statement> parseWindowUpdateStatement(
        const std::string& windowId
    );
    std::shared_ptr<Statement> parseWindowObjectStatement(
        const std::string& windowId,
        const std::string& method
    );
    UpdateAction parsePropertyAssignment();
    std::variant<std::string, UpdateAction::TextTemplate> parseTextValue();
    void parseElementProperty(ElementNode& element);
    std::vector<std::shared_ptr<Statement>> parseClickAction();

    std::vector<Token> tokens_;
    std::size_t position_ = 0;
};
