#include "token.hpp"

#include <iostream>

const char* Token::getName(){
    switch (type) {
    case TokenType::KEYWORD:
        return "keyword";
    case TokenType::IDENTIFIER:
        return "identifier";
    case TokenType::L_PAREN:
        return "(";
    case TokenType::R_PAREN:
        return ")";
    case TokenType::L_BRACK:
        return "[";
    case TokenType::R_BRACK:
        return "]";
    case TokenType::L_BRACE:
        return "{";
    case TokenType::R_BRACE:
        return "}";
    case TokenType::ASSIGN:
        return "=";
    case TokenType::NUMBER:
        return "number";
    case TokenType::NUMBER_HEX:
        return "number_hex";
    case TokenType::SEMICOLON:
        return ";";
    case TokenType::COMMA:
        return ",";
    case TokenType::END_OF_FILE:
        return "EOF";
    default:
        return "ERROR";
    }
}

void printToken(std::vector<Token> tokens) {
    for(int i = 0; i < tokens.size(); i++) {
        std::cout << tokens[i].getName() << " ";
    }

    std::cout << std::endl;
}
