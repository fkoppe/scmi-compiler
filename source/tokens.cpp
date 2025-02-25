#include "tokens.hpp"

#include <iostream>

#include "tokens.hpp"
#include <utility>
#include <sstream>

TypeType toTypeType(string name)
{
    if(name == "void") {
        return TypeType::VOID;
    }
    if(name == "char") {
        return TypeType::CHAR;
    }
    if(name == "short") {
        return TypeType::SHORT;
    }
    if(name == "float") {
        return TypeType::FLOAT;
    }
    if(name == "double") {
        return TypeType::DOUBLE;
    }
    if(name == "int") {
        return TypeType::INT;
    }

    cout << "Unknown TypeType: " << name << endl;
    exit(-1);
}

KeywordType toKeywordType(string name)
{
    if(name == "return") {
        return KeywordType::RETURN;
    }
    if(name == "if") {
        return KeywordType::IF;
    }
    if(name == "else") {
        return KeywordType::ELSE;
    }
    if(name == "while") {
        return KeywordType::WHILE;
    }
    if(name == "for") {
        return KeywordType::FOR;
    }

    return KeywordType::TYPE;
}

Token::Token(TokenType type_) {
    type = type_;
}

const char* Token::getTypeName(){
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

string Token::where() {
    stringstream ss;

    ss << "(line " << line << ":" << num << ")";

    return ss.str();
}

void printToken(vector<Token> tokens) {
    for(int i = 0; i < tokens.size(); i++) {
        cout << tokens[i].getTypeName() << " ";
    }

    cout << endl;
}
