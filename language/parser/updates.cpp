#include "../parser.h"

#include "parser_support.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <unordered_set>
#include <utility>

UpdateAction Parser::parsePropertyAssignment() {

    UpdateAction action;
    action.targetId = consume(
        TokenKind::Identifier,
        "Expected target element id."
    ).text;

    if (match(TokenKind::Dot)) {
        action.property = consume(
            TokenKind::Identifier,
            "Expected property name after '.'."
        ).text;
    }

    consume(TokenKind::Equals, "Expected '=' in update action.");

    if (action.targetId == "mouse" && action.property == "shape") {
        const std::string shape = consume(
            TokenKind::Identifier,
            "Expected mouse shape."
        ).text;

        static const std::vector<std::string> validShapes = {
            "ARROW", "IBEAM", "WAIT", "CROSS", "UPARROW", "SIZE", "ICON",
            "SIZENWSE", "SIZENESW", "SIZEWE", "SIZENS", "SIZEALL",
            "NO", "HAND", "APPSTARTING", "HELP", "PIN", "PERSON"
        };

        if (std::find(validShapes.begin(), validShapes.end(), shape) ==
            validShapes.end()) {
            throw std::runtime_error("Unknown mouse shape.");
        }

        action.value = shape;
    } else if (action.property == "background") {
        ColorNode color;
        color.red = consumeNumber("Expected red channel.");
        color.green = consumeNumber("Expected green channel.");
        color.blue = consumeNumber("Expected blue channel.");

        if (color.red < 0 || color.red > 255 ||
            color.green < 0 || color.green > 255 ||
            color.blue < 0 || color.blue > 255) {
            throw std::runtime_error(
                "Background color channels must be between 0 and 255."
            );
        }

        action.value = color;
    } else if (action.property == "x" || action.property == "y" ||
               action.property == "width" || action.property == "height" ||
               action.property == "angle" || action.property == "radius") {
        action.value = parseValueExpression();
    } else if (action.property == "fontSize") {
        action.value = parseValueExpression();
    } else if (action.property == "font") {
        action.value = consume(
            TokenKind::Identifier,
            "Expected font constant."
        ).text;
        if (!isFontConstant(std::get<std::string>(action.value))) {
            throw std::runtime_error("Unknown font constant.");
        }
    } else {
        auto text = parseTextValue();
        if (std::holds_alternative<UpdateAction::TextTemplate>(text)) {
            action.value = std::get<UpdateAction::TextTemplate>(text);
        } else {
            action.value = std::get<std::string>(text);
        }
    }

    consume(TokenKind::Semicolon, "Expected ';' after assignment.");

    return action;
}

std::variant<std::string, UpdateAction::TextTemplate> Parser::parseTextValue() {
    if (current().kind == TokenKind::Identifier && current().text == "f") {
        consume(TokenKind::Identifier, "Expected formatted string prefix.");
        return UpdateAction::TextTemplate{
            consumeString("Expected formatted string.")
        };
    }

    return consumeString("Expected replacement text.");
}

