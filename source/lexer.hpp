//
// Created by cedric on 20.02.25.
//

#ifndef LEXER_HPP
#define LEXER_HPP
#include "tokens.hpp"

using namespace std;
#include <string>
#include <vector>
#include <sstream>

std::string readFile(const std::string& path);

vector<Token> lexString(const string& data);
string removeWhitespace(const string& word);
void replaceAll(string& result, const string& from, const string& to);
vector<string> split(const string& str, char delimiter);
TokenType getToken(const string& word);


#endif //LEXER_HPP
