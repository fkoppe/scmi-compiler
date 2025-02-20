//
// Created by cedric on 20.02.25.
//

#ifndef LEXER_HPP
#define LEXER_HPP
#include "tokens.hpp"

using namespace std;
#include <string>;
#include <vector>;

string readFile(const string& path);

vector<TokenType> lexString(const string& data);




#endif //LEXER_HPP
