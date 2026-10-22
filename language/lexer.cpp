#include "lexer.h"

#include <cctype>
#include <stdexcept>
#include <string>
#include <utility>

Lexer::Lexer(std::string source)
    : source_(std::move(source)) {
}

std::vector<Token> Lexer::tokenize() const {
    std::vector<Token> tokens;
    std::size_t index = 0;
    int line = 1;
    int column = 1;

    // Some editors save UTF-8 files with a byte-order mark. It is metadata,
    // not part of the PXL program, so ignore it if it appears at the start.
    if (source_.size() >= 3 &&
        static_cast<unsigned char>(source_[0]) == 0xEF &&
        static_cast<unsigned char>(source_[1]) == 0xBB &&
        static_cast<unsigned char>(source_[2]) == 0xBF) {
        index = 3;
    }

    auto advance = [&] {
        if (source_[index] == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }
        ++index;
    };

    while (index < source_.size()) {
        char current = source_[index];

        if (std::isspace(static_cast<unsigned char>(current))) {
            advance();
            continue;
        }

        if (current == '#') {
            while (index < source_.size() && source_[index] != '\n') {
                advance();
            }
            continue;
        }

        int tokenLine = line;
        int tokenColumn = column;

        if (std::isalpha(static_cast<unsigned char>(current)) || current == '_') {
            std::string value;

            while (index < source_.size()) {
                char character = source_[index];

                if (!std::isalnum(static_cast<unsigned char>(character)) &&
                    character != '_') {
                    break;
                }

                value.push_back(character);
                advance();
            }

            tokens.push_back({
                TokenKind::Identifier,
                value,
                tokenLine,
                tokenColumn
            });
            continue;
        }

        if (current == '-' && index + 1 < source_.size() &&
            source_[index + 1] == '-') {
            advance();
            advance();
            tokens.push_back({
                TokenKind::Decrement,
                "--",
                tokenLine,
                tokenColumn
            });
            continue;
        }

        const bool unaryMinus = current == '-' &&
            index + 1 < source_.size() &&
            std::isdigit(static_cast<unsigned char>(source_[index + 1])) &&
            (tokens.empty() ||
             tokens.back().kind == TokenKind::Equals ||
             tokens.back().kind == TokenKind::OpenParen ||
             tokens.back().kind == TokenKind::Comma ||
             tokens.back().kind == TokenKind::Colon ||
             tokens.back().kind == TokenKind::Plus ||
             tokens.back().kind == TokenKind::Minus ||
             tokens.back().kind == TokenKind::EqualEqual ||
             tokens.back().kind == TokenKind::NotEqual ||
             tokens.back().kind == TokenKind::Less ||
             tokens.back().kind == TokenKind::LessEqual ||
             tokens.back().kind == TokenKind::Greater ||
             tokens.back().kind == TokenKind::GreaterEqual);

        if (std::isdigit(static_cast<unsigned char>(current)) || unaryMinus) {
            std::string value;

            if (current == '-') {
                value.push_back(current);
                advance();
            }

            while (index < source_.size() &&
                   std::isdigit(static_cast<unsigned char>(source_[index]))) {
                value.push_back(source_[index]);
                advance();
            }

            if (index < source_.size() && source_[index] == '.') {
                value.push_back('.');
                advance();

                if (index >= source_.size() ||
                    !std::isdigit(static_cast<unsigned char>(source_[index]))) {
                    throw std::runtime_error("Expected digits after decimal point.");
                }

                while (index < source_.size() &&
                       std::isdigit(static_cast<unsigned char>(source_[index]))) {
                    value.push_back(source_[index]);
                    advance();
                }
            }

            tokens.push_back({
                TokenKind::Number,
                value,
                tokenLine,
                tokenColumn
            });
            continue;
        }

        if (current == '"') {
            std::string value;
            advance();

            while (index < source_.size() && source_[index] != '"') {
                if (source_[index] == '\\') {
                    advance();

                    if (index >= source_.size()) {
                        throw std::runtime_error("Unterminated string.");
                    }

                    switch (source_[index]) {
                        case 'n': value.push_back('\n'); break;
                        case 't': value.push_back('\t'); break;
                        case '"': value.push_back('"'); break;
                        case '\\': value.push_back('\\'); break;
                        default:
                    throw std::runtime_error(
                        "Unknown string escape at line " +
                        std::to_string(tokenLine) + ".");
                    }

                    advance();
                } else {
                    value.push_back(source_[index]);
                    advance();
                }
            }

            if (index >= source_.size()) {
                throw std::runtime_error("Unterminated string.");
            }

            advance();
            tokens.push_back({
                TokenKind::String,
                value,
                tokenLine,
                tokenColumn
            });
            continue;
        }

        if (current == '\'') {
            char value;
            advance();

            if (index >= source_.size()) {
                throw std::runtime_error("Unterminated character literal.");
            }

            if (source_[index] == '\\') {
                advance();

                if (index >= source_.size()) {
                    throw std::runtime_error("Unterminated character literal.");
                }

                switch (source_[index]) {
                    case 'n': value = '\n'; break;
                    case 't': value = '\t'; break;
                    case '\'': value = '\''; break;
                    case '\\': value = '\\'; break;
                    default:
                        throw std::runtime_error(
                            "Unknown character escape at line " +
                            std::to_string(tokenLine) + ".");
                }
            } else {
                value = source_[index];
            }

            advance();

            if (index >= source_.size() || source_[index] != '\'') {
                throw std::runtime_error("Character literals must contain one character.");
            }

            advance();
            tokens.push_back({
                TokenKind::Character,
                std::string(1, value),
                tokenLine,
                tokenColumn
            });
            continue;
        }

        TokenKind kind;

        switch (current) {
            case ':': kind = TokenKind::Colon; break;
            case '.': kind = TokenKind::Dot; break;
            case '=':
                if (index + 1 < source_.size() && source_[index + 1] == '=') {
                    kind = TokenKind::EqualEqual;
                    advance();
                } else {
                    kind = TokenKind::Equals;
                }
                break;
            case '!':
                if (index + 1 < source_.size() && source_[index + 1] == '=') {
                    kind = TokenKind::NotEqual;
                    advance();
                } else {
                    kind = TokenKind::Bang;
                }
                break;
            case '<':
                if (index + 1 < source_.size() && source_[index + 1] == '=') {
                    kind = TokenKind::LessEqual;
                    advance();
                } else {
                    kind = TokenKind::Less;
                }
                break;
            case '>':
                if (index + 1 < source_.size() && source_[index + 1] == '=') {
                    kind = TokenKind::GreaterEqual;
                    advance();
                } else {
                    kind = TokenKind::Greater;
                }
                break;
            case '&':
                if (index + 1 < source_.size() && source_[index + 1] == '&') {
                    kind = TokenKind::AndAnd;
                    advance();
                } else {
                    throw std::runtime_error("Expected '&' in '&&'.");
                }
                break;
            case '|':
                if (index + 1 < source_.size() && source_[index + 1] == '|') {
                    kind = TokenKind::OrOr;
                    advance();
                } else {
                    throw std::runtime_error("Expected '|' in '||'.");
                }
                break;
            case '+':
                if (index + 1 < source_.size() && source_[index + 1] == '+') {
                    kind = TokenKind::Increment;
                    advance();
                } else {
                    kind = TokenKind::Plus;
                }
                break;
            case '-': kind = TokenKind::Minus; break;
            case '*': kind = TokenKind::Star; break;
            case '/': kind = TokenKind::Slash; break;
            case '%': kind = TokenKind::Percent; break;
            case '(': kind = TokenKind::OpenParen; break;
            case ')': kind = TokenKind::CloseParen; break;
            case ';': kind = TokenKind::Semicolon; break;
            case ',': kind = TokenKind::Comma; break;
            case '{': kind = TokenKind::OpenBrace; break;
            case '}': kind = TokenKind::CloseBrace; break;
            default:
                throw std::runtime_error(
                    "Unexpected character '" + std::string(1, current) +
                    "' at line " + std::to_string(tokenLine) +
                    ", column " + std::to_string(tokenColumn) + ".");
        }

        advance();
        tokens.push_back({kind, std::string(1, current), tokenLine, tokenColumn});
    }

    tokens.push_back({TokenKind::End, {}, line, column});
    return tokens;
}
