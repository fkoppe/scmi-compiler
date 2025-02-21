//
// Created by cedric on 20.02.25.
//

#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <string>
#include <vector>
using namespace std;

enum TokenType {
    KEYWORD,
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
