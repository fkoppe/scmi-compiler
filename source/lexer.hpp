#ifndef LEXER_HPP
#define LEXER_HPP

#include "token.hpp"

#include <string>
#include <vector>
#include <sstream>

using namespace std;

class Lexer {
public:
    vector<Token> lexText(const string& text);
private:
    uint64_t line = 1;
    uint64_t num = 0;
    string word;
    vector<Token> result;

    void processWord();
    void lexString(string str);
};

string readFile(const string& path);
TokenType getToken(const string& word);

#endif //LEXER_HPP
