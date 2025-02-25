#ifndef LEXER_HPP
#define LEXER_HPP

#include "tokens.hpp"

#include <string>
#include <vector>
#include <sstream>

const std::string INLINE_COMMENT = "//";

std::string readFile(const std::string& path);

std::vector<Token> lexString(const std::string& data);
std::string removeWhitespace(const std::string& word);
void replaceAll(std::string& result, const std::string& from, const std::string& to);
std::vector<std::string> split(const std::string& str, char delimiter);
TokenType getToken(const std::string& word);


#endif //LEXER_HPP
