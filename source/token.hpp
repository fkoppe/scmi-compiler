//
// Created by cedric on 20.02.25.
//

#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <string>
#include <vector>
#include <unordered_set>
#include <cstdint>
#include <variant>

#include "Keyword.hpp"

using namespace std;

const unordered_set<string> SPECIAL_SET = {
    "true",
    "false",
};

const unordered_set<string> TYPE_SET = {
    "void",
    "int",
    "short",
    "char",
    "float",
    "double",
};

const unordered_set<string> KEYWORD_SET = {
    "void",
    "int",
    "short",
    "char",
    "float",
    "double",
    "return",
    "if",
    "else",
    "while",
    "for",
    "goto",
};

enum class KeywordType {
    TYPE,
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
    SPECIAL,
    KEYWORD,
    IDENTIFIER,
    L_PAREN,
    R_PAREN,
    L_BRACK, //[
    R_BRACK, //]
    L_BRACE,
    R_BRACE,
    ASSIGN,
    NUMBER,
    SEMICOLON,
    COMMA,
    NOT,
    GREATER,
    LESS,
    AND,
    OR,
    ADD,
    SUB,
    MULT,
    DIV,
    MOD,
    END_OF_FILE,
    LABEL,
};

TypeType toTypeType(string name);
KeywordType toKeywordType(string name);

class Token {
public:
    Token(TokenType type);

    TokenType type;

    uint64_t line = 0;
    uint64_t num = 0;

    string raw;

    KeywordType keyword;
    NumberType number;

    const char* getTypeName();

    string where();
};

static const Token eof = Token(TokenType::END_OF_FILE);

void printToken(vector<Token> tokens);

#endif //TOKENS_HPP
