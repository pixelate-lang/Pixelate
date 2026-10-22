#pragma once

#include <optional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

enum class ElementType {
    Label,
    Paragraph,
    Button,
    TextBox,
    TextArea,
    Rect,
    Circle,
    Triangle
};

struct PointNode {
    int x = 0;
    int y = 0;
};

enum class VariableType {
    Int,
    Short,
    Long,
    Float,
    Double,
    Bool,
    Char,
    String
};

using VariableValue = std::variant<
    int,
    short,
    long long,
    float,
    double,
    bool,
    char,
    std::string
>;

struct VariableDeclaration {
    VariableType type;
    std::string name;
    VariableValue value;
};

struct ArithmeticExpression;

struct ValueExpression {
    std::optional<VariableValue> literal;
    std::string variable;
    std::shared_ptr<ArithmeticExpression> arithmetic;
};

struct ArithmeticExpression {
    ValueExpression left;
    ValueExpression right;
    char operation = '+';
};

struct VariableAssignment {
    std::string name;
    ValueExpression value;
};

struct FormattedPart {
    std::variant<std::string, ValueExpression> value;
};

struct PrintStatement {
    bool newline = false;
    bool formatted = false;
    ValueExpression expression;
    std::vector<FormattedPart> parts;
};

enum class ComparisonOperator {
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual
};

struct Condition {
    ValueExpression left;
    ComparisonOperator operation;
    ValueExpression right;
    bool hasComparison = true;
    bool negated = false;
    enum class LogicalOperator { None, And, Or } logical = LogicalOperator::None;
    std::shared_ptr<Condition> leftCondition;
    std::shared_ptr<Condition> rightCondition;
};

struct ColorNode {
    int red = 255;
    int green = 255;
    int blue = 255;
};

struct UpdateAction {
    std::string targetId;
    std::string property = "text";
    struct TextTemplate {
        std::string text;
    };
    std::variant<int, std::string, ColorNode, TextTemplate, ValueExpression> value;
};

struct Statement;

struct IfStatement {
    Condition condition;
    std::vector<std::shared_ptr<Statement>> body;
    std::vector<std::shared_ptr<Statement>> elseBody;
};

struct WhileStatement {
    Condition condition;
    std::vector<std::shared_ptr<Statement>> body;
};

struct ForStatement {
    VariableDeclaration initializer;
    Condition condition;
    std::string incrementVariable;
    std::vector<std::shared_ptr<Statement>> body;
};

struct IncrementStatement {
    std::string target;
    std::string property;
    bool decrement = false;
};

struct SwitchViewStatement {
    std::string windowId;
};

struct AddViewStatement {
    std::string windowId;
};

struct TriggerStatement {
    std::string id;
    bool mouseMove = false;
    bool closeTrigger = false;
    bool attached = true;
    std::optional<std::string> key;
    bool keyCaseInsensitive = false;
    std::optional<Condition> condition;
    std::vector<std::shared_ptr<Statement>> body;
};

struct WindowUpdateStatement {
    std::string windowId;
};

struct WindowObjectStatement {
    std::string windowId;
    std::string objectId;
    bool add = true;
};

struct Statement {
    std::variant<
        VariableDeclaration,
        VariableAssignment,
        PrintStatement,
        UpdateAction,
        IncrementStatement,
        SwitchViewStatement,
        AddViewStatement,
        WindowUpdateStatement,
        WindowObjectStatement,
        std::shared_ptr<IfStatement>,
        std::shared_ptr<WhileStatement>,
        std::shared_ptr<ForStatement>,
        std::shared_ptr<TriggerStatement>
    > value;
    int line = 0;
};

struct ElementNode {
    ElementType type;
    std::string id;
    std::variant<std::string, UpdateAction::TextTemplate> text;
    std::string placeholder;
    std::string font = "DEFAULT";
    int fontSize = 16;
    int x = 0;
    int y = 0;
    bool xSpecified = false;
    bool ySpecified = false;
    int width = 100;
    int height = 30;
    int angle = 0;
    int radius = 10;
    PointNode p1;
    PointNode p2;
    PointNode p3;
    bool p1Specified = false;
    bool p2Specified = false;
    bool p3Specified = false;
    std::optional<std::vector<std::shared_ptr<Statement>>> onClick;
};

struct WindowNode {
    std::string id;
    std::string title = "Pixelate App";
    int width = 800;
    int height = 600;
    std::string iconPath = "pixelate.ico";
    ColorNode background;
    std::vector<ElementNode> elements;
    std::vector<std::shared_ptr<TriggerStatement>> triggers;
};

struct ProgramNode {
    std::vector<VariableDeclaration> variables;
    std::vector<std::shared_ptr<Statement>> statements;
    std::vector<std::shared_ptr<TriggerStatement>> triggers;
    std::vector<ElementNode> elements;
    std::vector<WindowNode> windows;
    std::string entryWindow;
};
