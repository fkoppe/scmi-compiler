//
// Created by cedric on 20.02.25.
//

#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <string>
#include <vector>
#include <unordered_set>

using namespace std;

const unordered_set<string> KEYWORD_SET = {
    "void",
    "int",
    "short",
    "char",
    "float",
    "double",
};

const unordered_set<string> CONTROL_SET = {
    "return",
    "if",
    "else",
    "while",
    "for",
};

enum TokenType {
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
    NUMBER_HEX,
    SEMICOLON,
    COMMA,
    END_OF_FILE,
};

struct Token {
    TokenType type;
    string value;
};

const char* tokenTypeName(TokenType name);
void printTokens(std::vector<Token> tokens);

#endif //TOKENS_HPP
