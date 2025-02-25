//
// Created by cedric on 20.02.25.
//

#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <string>
#include <vector>
#include <unordered_set>

const std::unordered_set<std::string> KEYWORD_SET = {
    "void",
    "int",
    "short",
    "char",
    "float",
    "double",
};

enum class KeywordType {
    VOID,
    INT,
    SHORT,
    CHAR,
    FLOAT,
    DOUBLE,
};

const std::unordered_set<std::string> CONTROL_SET = {
    "return",
    "if",
    "else",
    "while",
    "for",
};

enum class ControlType {
    RETURN,
    IF,
    ELSE,
    WHILE,
    FOR,
};

enum class NumberType {
    DECIMAL,
    HEX,
};

enum class TokenType {
    KEYWORD,
    CONTROL,
    IDENTIFIER,
    L_PAREN,
    R_PAREN,
    L_BRACK,
    R_BRACK,
    L_BRACE,
    R_BRACE,
    ASSIGN,
    NUMBER,
    SEMICOLON,
    COMMA,
    END_OF_FILE,
};

struct Token {
    Token() = default;
    ~Token() = default;

    TokenType type;

    uint64_t line;
    uint64_t num;

    union {
        struct {
            KeywordType type;
        } keyword;
        struct {
            ControlType type;
        } control;
        struct {
            std::string name;
        } identifier;
        struct {
            NumberType number;
        } number;
    };

    const char* getName();
};

void printToken(std::vector<Token> tokens);

#endif //TOKENS_HPP
