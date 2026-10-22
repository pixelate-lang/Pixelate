#include "../parser.h"

#include "parser_support.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <unordered_set>
#include <utility>

Parser::Parser(std::vector<Token> tokens)
    : tokens_(std::move(tokens)) {
}

const Token& Parser::current() const {
    return tokens_.at(position_);
}

int Parser::currentLine() const {
    return current().line;
}

const Token& Parser::peek(std::size_t offset) const {
    return tokens_.at(position_ + offset);
}

bool Parser::match(TokenKind kind) {
    if (current().kind != kind) {
        return false;
    }

    ++position_;
    return true;
}

Token Parser::consume(TokenKind kind, const char* message) {
    if (current().kind != kind) {
        throw std::runtime_error(message);
    }

    return tokens_.at(position_++);
}

void Parser::consumeFieldSeparator(const char* message) {
    // Fields are comma-delimited, but the final field may be followed
    // directly by the closing brace (like a Python dictionary literal).
    if (current().kind == TokenKind::CloseBrace) {
        return;
    }

    consume(TokenKind::Comma, message);
}

void Parser::consumeIdentifier(const char* value) {
    Token token = consume(TokenKind::Identifier, "Expected identifier.");

    if (token.text != value) {
        throw std::runtime_error("Unexpected identifier.");
    }
}

std::string Parser::consumeString(const char* message) {
    return consume(TokenKind::String, message).text;
}

int Parser::consumeNumber(const char* message) {
    Token token = consume(TokenKind::Number, message);

    try {
        return std::stoi(token.text);
    } catch (...) {
        throw std::runtime_error("Invalid number.");
    }
}

ProgramNode Parser::parse() {
    ProgramNode program;

    while (current().kind != TokenKind::End) {
        Token declaration = consume(
            TokenKind::Identifier,
            "Expected a declaration or statement."
        );

        if (declaration.text == "window") {
            program.windows.push_back(parseWindowBlock());
        } else if (declaration.text == "label" ||
                   declaration.text == "par" ||
                   declaration.text == "button" ||
                   declaration.text == "textbox" ||
                   declaration.text == "textarea" ||
                   declaration.text == "rect" ||
                   declaration.text == "circle" ||
                   declaration.text == "triangle") {
            --position_;
            parseElementIntoProgram(program);
        } else if (
            declaration.text == "int" ||
            declaration.text == "short" ||
            declaration.text == "long" ||
            declaration.text == "float" ||
            declaration.text == "double" ||
            declaration.text == "bool" ||
            declaration.text == "char" ||
            declaration.text == "String"
        ) {
            VariableDeclaration variable = parseVariableDeclaration(
                declaration.text
            );
            program.variables.push_back(variable);
            auto statement = std::make_shared<Statement>();
            statement->value = variable;
            statement->line = declaration.line;
            program.statements.push_back(std::move(statement));
        } else if (
            current().kind == TokenKind::Dot &&
            peek(1).kind == TokenKind::Identifier &&
            peek(2).kind == TokenKind::OpenParen
        ) {
            --position_;
            parseObjectMethod(program);
        } else if (declaration.text == "trigger") {
            --position_;
            parseTriggerDeclaration(program);
        } else {
            --position_;
            program.statements.push_back(parseStatement());
        }
    }

    consume(TokenKind::End, "Unexpected content after program.");

    if (program.windows.size() > 1) {
        std::unordered_set<std::string> ids;
        for (const WindowNode& window : program.windows) {
            if (window.id.empty()) {
                throw std::runtime_error(
                    "Every window needs an id when multiple windows are defined."
                );
            }

            if (!ids.insert(window.id).second) {
                throw std::runtime_error(
                    "Window ids must be unique: " + window.id
                );
            }
        }
    }

    return program;
}

WindowNode Parser::parseWindowBlock() {
    WindowNode window;

    Token name = consume(TokenKind::Identifier, "Expected window name.");
    consume(TokenKind::Equals, "Expected '=' after window name.");
    consumeIdentifier("window");
    window.id = name.text;

    consume(TokenKind::OpenBrace, "Expected '{' after window.");

    while (!match(TokenKind::CloseBrace)) {
        Token token = consume(
            TokenKind::Identifier,
            "Expected window property or element."
        );

        if (token.text == "title") {
            consume(TokenKind::Colon, "Expected ':' after title.");
            window.title = consumeString("Expected window title.");
            consumeFieldSeparator("Expected ',' after title.");
        } else if (token.text == "width") {
            consume(TokenKind::Colon, "Expected ':' after width.");
            window.width = consumeNumber("Expected window width.");
            consumeFieldSeparator("Expected ',' after width.");
        } else if (token.text == "height") {
            consume(TokenKind::Colon, "Expected ':' after height.");
            window.height = consumeNumber("Expected window height.");
            consumeFieldSeparator("Expected ',' after height.");
        } else if (token.text == "iconPath") {
            consume(TokenKind::Colon, "Expected ':' after iconPath.");
            window.iconPath = consumeString("Expected icon path.");
            consumeFieldSeparator("Expected ',' after iconPath.");
        } else if (token.text == "background") {
            consume(TokenKind::Colon, "Expected ':' after background.");
            window.background.red = consumeNumber("Expected red channel.");
            window.background.green = consumeNumber("Expected green channel.");
            window.background.blue = consumeNumber("Expected blue channel.");

            if (window.background.red < 0 || window.background.red > 255 ||
                window.background.green < 0 || window.background.green > 255 ||
                window.background.blue < 0 || window.background.blue > 255) {
                throw std::runtime_error(
                    "Background color channels must be between 0 and 255."
                );
            }
            consumeFieldSeparator("Expected ',' after background.");
        } else {
            throw std::runtime_error(
                "Unknown window property. Declare elements separately and add them with window.add(element)."
            );
        }
    }

    return window;
}

VariableDeclaration Parser::parseVariableDeclaration(const std::string& typeName) {
    VariableType type;

    if (typeName == "int") type = VariableType::Int;
    else if (typeName == "short") type = VariableType::Short;
    else if (typeName == "long") type = VariableType::Long;
    else if (typeName == "float") type = VariableType::Float;
    else if (typeName == "double") type = VariableType::Double;
    else if (typeName == "bool") type = VariableType::Bool;
    else if (typeName == "char") type = VariableType::Char;
    else if (typeName == "String") type = VariableType::String;
    else throw std::runtime_error("Unknown variable type.");

    std::string name = consume(
        TokenKind::Identifier,
        "Expected variable name."
    ).text;

    consume(TokenKind::Equals, "Expected '=' after variable name.");

    VariableValue value;

    try {
        if (type == VariableType::String) {
            value = consumeString("Expected string value.");
        } else if (type == VariableType::Char) {
            value = consume(TokenKind::Character, "Expected character value.").text[0];
        } else if (type == VariableType::Bool) {
            std::string boolean = consume(
                TokenKind::Identifier,
                "Expected true or false."
            ).text;

            if (boolean == "true") value = true;
            else if (boolean == "false") value = false;
            else throw std::runtime_error("Expected true or false.");
        } else {
            std::string number = consume(
                TokenKind::Number,
                "Expected numeric value."
            ).text;

            bool decimal = number.find('.') != std::string::npos;

            if ((type == VariableType::Int ||
                 type == VariableType::Short ||
                 type == VariableType::Long) && decimal) {
                throw std::runtime_error(
                    "Integer variables cannot use decimal values."
                );
            }

            if (type == VariableType::Int) value = std::stoi(number);
            else if (type == VariableType::Short) {
                value = static_cast<short>(std::stoi(number));
            } else if (type == VariableType::Long) {
                value = std::stoll(number);
            } else if (type == VariableType::Float) {
                value = static_cast<float>(std::stod(number));
            } else if (type == VariableType::Double) {
                value = std::stod(number);
            }
        }
    } catch (const std::invalid_argument&) {
        throw std::runtime_error("Invalid variable value.");
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Variable value is out of range.");
    }

    VariableDeclaration declaration{type, name, value};
    consume(TokenKind::Semicolon, "Expected ';' after variable declaration.");
    return declaration;
}

void Parser::parseElementIntoProgram(ProgramNode& program) {
    Token typeToken = consume(
        TokenKind::Identifier,
        "Expected an element type."
    );

    if (typeToken.text != "label" && typeToken.text != "par" &&
        typeToken.text != "button" && typeToken.text != "textbox" &&
        typeToken.text != "textarea" && typeToken.text != "rect" &&
        typeToken.text != "circle" && typeToken.text != "triangle") {
        throw std::runtime_error(
            "Unknown element type: " + typeToken.text
        );
    }

    Token id = consume(TokenKind::Identifier, "Expected element name.");
    consume(TokenKind::Equals, "Expected '=' after element name.");
    consumeIdentifier(typeToken.text.c_str());

    ElementType type = typeToken.text == "label"
        ? ElementType::Label
        : typeToken.text == "par"
            ? ElementType::Paragraph
        : typeToken.text == "button"
            ? ElementType::Button
            : typeToken.text == "textbox"
                ? ElementType::TextBox
                : typeToken.text == "textarea"
                    ? ElementType::TextArea
                    : typeToken.text == "rect"
                        ? ElementType::Rect
                        : typeToken.text == "circle"
                            ? ElementType::Circle
                            : ElementType::Triangle;

    ElementNode element = parseElement(type);
    element.id = id.text;
    program.elements.push_back(std::move(element));
}

void Parser::parseObjectMethod(ProgramNode& program) {
    const Token target = consume(
        TokenKind::Identifier,
        "Expected object name before method call."
    );
    consume(TokenKind::Dot, "Expected '.' in method call.");
    const Token method = consume(
        TokenKind::Identifier,
        "Expected method name."
    );
    consume(TokenKind::OpenParen, "Expected '(' after method name.");

    if (method.text == "add" || method.text == "remove") {
        const bool removing = method.text == "remove";
        const Token objectName = consume(
            TokenKind::Identifier,
            "Expected object name in add or remove call."
        );
        consume(TokenKind::CloseParen, "Expected ')' after add/remove argument.");
        consume(TokenKind::Semicolon, "Expected ';' after add/remove call.");

        auto window = std::find_if(
            program.windows.begin(),
            program.windows.end(),
            [&](const WindowNode& candidate) {
                return candidate.id == target.text;
            }
        );
        if (window == program.windows.end()) {
            throw std::runtime_error(
                "Unknown window object: " + target.text
            );
        }

        auto element = std::find_if(
            program.elements.begin(),
            program.elements.end(),
            [&](const ElementNode& candidate) {
                return candidate.id == objectName.text;
            }
        );
        if (element != program.elements.end()) {
            auto attachedElement = std::find_if(
                window->elements.begin(),
                window->elements.end(),
                [&](const ElementNode& candidate) {
                    return candidate.id == objectName.text;
                }
            );
            if (removing) {
                if (attachedElement == window->elements.end()) {
                    throw std::runtime_error(
                        "Element is not attached to window: " + objectName.text
                    );
                }
                window->elements.erase(attachedElement);
            } else {
                if (attachedElement != window->elements.end()) {
                    throw std::runtime_error(
                        "Element already added to window: " + objectName.text
                    );
                }
                window->elements.push_back(*element);
            }
            auto statement = std::make_shared<Statement>();
            statement->value = WindowUpdateStatement{target.text};
            statement->line = target.line;
            program.statements.push_back(std::move(statement));
            return;
        }

        auto trigger = std::find_if(
            program.triggers.begin(),
            program.triggers.end(),
            [&](const auto& candidate) {
                return candidate->id == objectName.text;
            }
        );
        if (trigger == program.triggers.end()) {
            throw std::runtime_error(
                "Unknown object in add or remove call: " + objectName.text
            );
        }
        auto attachedTrigger = std::find_if(
            window->triggers.begin(),
            window->triggers.end(),
            [&](const auto& candidate) {
                return candidate->id == objectName.text;
            }
        );
        if (removing) {
            if (attachedTrigger == window->triggers.end()) {
                throw std::runtime_error(
                    "Trigger is not attached to window: " + objectName.text
                );
            }
            window->triggers.erase(attachedTrigger);
        } else {
            if (attachedTrigger != window->triggers.end()) {
                throw std::runtime_error(
                    "Trigger already attached to window: " + objectName.text
                );
            }
            window->triggers.push_back(*trigger);
        }
        auto statement = std::make_shared<Statement>();
        statement->value = WindowUpdateStatement{target.text};
        statement->line = target.line;
        program.statements.push_back(std::move(statement));
        return;
    }

    if (method.text == "run") {
        consume(TokenKind::CloseParen, "Expected ')' after run.");
        consume(TokenKind::Semicolon, "Expected ';' after run call.");

        const auto window = std::find_if(
            program.windows.begin(),
            program.windows.end(),
            [&](const WindowNode& candidate) {
                return candidate.id == target.text;
            }
        );
        if (window == program.windows.end()) {
            throw std::runtime_error(
                "Unknown window object: " + target.text
            );
        }
        if (!program.entryWindow.empty()) {
            throw std::runtime_error(
                "Only one window may be selected with run()."
            );
        }

        program.entryWindow = target.text;
        return;
    }

    if (method.text == "update") {
        consume(TokenKind::CloseParen, "Expected ')' after update.");
        consume(TokenKind::Semicolon, "Expected ';' after update call.");

        auto statement = std::make_shared<Statement>();
        statement->value = WindowUpdateStatement{target.text};
        program.statements.push_back(std::move(statement));
        return;
    }

    throw std::runtime_error(
        "Unknown window method: " + method.text
    );
}

void Parser::parseTriggerDeclaration(ProgramNode& program) {
    consumeIdentifier("trigger");
    const Token name = consume(
        TokenKind::Identifier,
        "Expected trigger name."
    );
    consume(TokenKind::Equals, "Expected '=' after trigger name.");
    consumeIdentifier("trigger");

    auto trigger = parseTriggerBody();
    trigger->id = name.text;
    program.triggers.push_back(std::move(trigger));
}

