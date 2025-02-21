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

vector<TokenType> lexString(const std::string& data);
std::string removeWhitespace(const std::string& word);
void replaceAll(std::string& result, const std::string& from, const std::string& to);
vector<std::string> split(const std::string& str, char delimiter);
TokenType getToken(const std::string& word);


#endif //LEXER_HPP
