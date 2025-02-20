//
// Created by cedric on 20.02.25.
//

#include "lexer.hpp"

string readFile(const string& path) {

}

vector<TokenType> lexString(const string& data) {

}

TokenType getToken(const string& word) {
    switch (word) {
        case "int":
        case "short":
        case "char":
        case "void":
        case "float":
        case "double":
            return KEYWORD;
        case "{":
            return L_BRACK;

    }
}
