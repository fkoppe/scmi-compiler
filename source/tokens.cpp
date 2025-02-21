#include "tokens.hpp"

#include <iostream>

const char* tokenTypeName(TokenType name){
    switch (name) {
    case KEYWORD:
        return "keyword";
    case IDENTIFIER:
        return "identifier";
    case L_PAREN:
        return "(";
    case R_PAREN:
        return ")";
    case L_BRACK:
        return "[";
    case R_BRACK:
        return "]";
    case L_BRACE:
        return "{";
    case R_BRACE:
        return "}";
    case ASSIGN:
        return "=";
    case NUMBER:
        return "number";
    case SEMICOLON:
        return ";";
    case COMMA:
        return ",";
    default:
        return "ERROR";
    }
}

void printTokens(std::vector<Token> tokens) {
    for(int i = 0; i < tokens.size(); i++) {
        std::cout << tokenTypeName(tokens[i].type) << " ";
    }

    std::cout << std::endl;
}
