//
// Created by cedric on 20.02.25.
//

#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <vector>

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
    SEMICOLON,
    COMMA,
};

const char* tokenTypeName(TokenType name);
void printTokens(std::vector<TokenType> tokens);

#endif //TOKENS_HPP
