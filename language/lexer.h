#pragma once

#include <string>
#include <vector>

enum class TokenKind {
    Identifier,
    String,
    Character,
    Number,
    Colon,
    Dot,
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Decrement,
    Equals,
    Bang,
    EqualEqual,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    AndAnd,
    OrOr,
    Increment,
    OpenParen,
    CloseParen,
    Semicolon,
    Comma,
    OpenBrace,
    CloseBrace,
    End
};

struct Token {
    TokenKind kind;
    std::string text;
    int line;
    int column;
};

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize() const;

private:
    std::string source_;
};
