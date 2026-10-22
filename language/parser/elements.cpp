#include "../parser.h"

#include "parser_support.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <unordered_set>
#include <utility>

ElementNode Parser::parseElement(ElementType type) {
    ElementNode element{};
    element.type = type;

    consume(TokenKind::OpenBrace, "Expected '{' after element id.");

    while (!match(TokenKind::CloseBrace)) {
        parseElementProperty(element);
    }

    if (type == ElementType::Triangle) {
        const bool anyPoint = element.p1Specified || element.p2Specified || element.p3Specified;
        const bool allPoints = element.p1Specified && element.p2Specified && element.p3Specified;
        if (anyPoint && !allPoints) {
            throw std::runtime_error("Triangle elements must specify p1, p2, and p3 together.");
        }
        if (element.xSpecified != element.ySpecified) {
            throw std::runtime_error("Triangle elements must specify both x and y together.");
        }
    }

    return element;
}

void Parser::parseElementProperty(ElementNode& element) {
    Token property = consume(TokenKind::Identifier, "Expected element property.");

    if (property.text == "text") {
        consume(TokenKind::Colon, "Expected ':' after text.");
        element.text = parseTextValue();
        consumeFieldSeparator("Expected ',' after text.");
    } else if (property.text == "placeholder") {
        if (element.type != ElementType::TextBox &&
            element.type != ElementType::TextArea) {
            throw std::runtime_error(
                "Only textbox and textarea elements can use placeholder."
            );
        }
        consume(TokenKind::Colon, "Expected ':' after placeholder.");
        element.placeholder = consumeString("Expected placeholder text.");
        consumeFieldSeparator("Expected ',' after placeholder.");
    } else if (property.text == "x") {
        consume(TokenKind::Colon, "Expected ':' after x.");
        element.x = consumeNumber("Expected x position.");
        element.xSpecified = true;
        consumeFieldSeparator("Expected ',' after x.");
    } else if (property.text == "y") {
        consume(TokenKind::Colon, "Expected ':' after y.");
        element.y = consumeNumber("Expected y position.");
        element.ySpecified = true;
        consumeFieldSeparator("Expected ',' after y.");
    } else if (property.text == "width") {
        consume(TokenKind::Colon, "Expected ':' after width.");
        element.width = consumeNumber("Expected element width.");
        consumeFieldSeparator("Expected ',' after width.");
    } else if (property.text == "height") {
        consume(TokenKind::Colon, "Expected ':' after height.");
        element.height = consumeNumber("Expected element height.");
        consumeFieldSeparator("Expected ',' after height.");
    } else if (property.text == "angle") {
        if (element.type != ElementType::Rect && element.type != ElementType::Triangle) {
            throw std::runtime_error("Only rect and triangle elements can use angle.");
        }
        consume(TokenKind::Colon, "Expected ':' after angle.");
        element.angle = consumeNumber("Expected rectangle angle.");
        consumeFieldSeparator("Expected ',' after angle.");
    } else if (property.text == "radius") {
        if (element.type != ElementType::Circle) throw std::runtime_error("Only circle elements can use radius.");
        consume(TokenKind::Colon, "Expected ':' after radius.");
        element.radius = consumeNumber("Expected circle radius.");
        if (element.radius <= 0) throw std::runtime_error("Circle radius must be greater than zero.");
        consumeFieldSeparator("Expected ',' after radius.");
    } else if (property.text == "p1" || property.text == "p2" || property.text == "p3") {
        if (element.type != ElementType::Triangle) throw std::runtime_error("Only triangle elements can use points.");
        consume(TokenKind::Colon, "Expected ':' after triangle point.");
        consume(TokenKind::OpenParen, "Expected '(' before triangle point.");
        PointNode point;
        point.x = consumeNumber("Expected triangle point x.");
        consume(TokenKind::Comma, "Expected ',' between triangle point coordinates.");
        point.y = consumeNumber("Expected triangle point y.");
        consume(TokenKind::CloseParen, "Expected ')' after triangle point.");
        if (property.text == "p1") { element.p1 = point; element.p1Specified = true; }
        if (property.text == "p2") { element.p2 = point; element.p2Specified = true; }
        if (property.text == "p3") { element.p3 = point; element.p3Specified = true; }
        consumeFieldSeparator("Expected ',' after triangle point.");
    } else if (property.text == "font") {
        consume(TokenKind::Colon, "Expected ':' after font.");
        element.font = consume(TokenKind::Identifier, "Expected font constant.").text;
        if (!isFontConstant(element.font)) {
            throw std::runtime_error("Unknown font constant.");
        }
        consumeFieldSeparator("Expected ',' after font.");
    } else if (property.text == "fontSize") {
        consume(TokenKind::Colon, "Expected ':' after fontSize.");
        element.fontSize = consumeNumber("Expected font size.");
        if (element.fontSize <= 0) {
            throw std::runtime_error("Font size must be greater than zero.");
        }
        consumeFieldSeparator("Expected ',' after fontSize.");
    } else if (property.text == "onclick") {
        element.onClick = parseClickAction();
        consumeFieldSeparator("Expected ',' after onclick.");
    } else {
        throw std::runtime_error("Unknown element property.");
    }
}

std::vector<std::shared_ptr<Statement>> Parser::parseClickAction() {
    consume(TokenKind::OpenBrace, "Expected '{' after onclick.");

    std::vector<std::shared_ptr<Statement>> actions;

    while (!match(TokenKind::CloseBrace)) {
        actions.push_back(parseStatement());
    }

    return actions;
}

