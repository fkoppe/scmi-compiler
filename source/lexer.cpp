//
// Created by cedric on 20.02.25.
//

#include "lexer.hpp"

#include <fstream>
#include <string>
#include <iostream>

string readFile(const string& path) {
    ifstream file(path, std::ios::ate);  // Open in "ate" mode to get file size
    if (!file) {
        throw std::runtime_error("Could not open file: " + path);
    }
    const streamsize size = file.tellg();  // Get file size
    file.seekg(0);  // Move back to the beginning of the file
    string content = string(size, '\0');
    file.read(content.data(), size);  // Read file content into string
    return content;
}

vector<Token> lexString(const string& data) {
    const string whiteSpaceData = removeWhitespace(data);
    vector<string> splitData = split(whiteSpaceData, ' ');

    vector<Token> result;
    result.reserve(splitData.size());

    for (const string& x : splitData) {
        Token token = {getToken(x),x};
        result.push_back(token);
    }

    return result;
}

vector<string> split(const string& str, char delimiter) {
    vector<string> result;
    string word;
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == delimiter) {
            result.push_back(word);
            word = "";
        }
        else {
            word += str[i];
        }
    }
    return result;
}

TokenType getToken(const string& word) {
    if(KEYWORD_SET.count(word)) {
        return KEYWORD;
    }

    if(CONTROL_SET.count(word)) {
        return CONTROL;
    }

    switch (word[0]) {
        case '{':
            return L_BRACE;
        case '}':
            return R_BRACE;
        case '(':
            return L_PAREN;
        case ')':
            return R_PAREN;
        case '[':
            return L_BRACK;
        case ']':
            return R_BRACK;
        case '=':
            return ASSIGN;
        case ';':
            return SEMICOLON;
        case ',':
            return COMMA;
    };

    if(isalpha(word[0])) {
        return IDENTIFIER;
    }

    if(isdigit(word[0])) {
        bool hex = false;


        if (word[1] == 'x') {
            hex = true;
        }

        bool digit = true;
        for (int i = hex ? 2 : 1; i < word.length(); i++) {
            if (!isdigit(word[i])) {
                digit = false;
                break;
            }
        }

        if (digit) {
            if (hex) {
                return NUMBER_HEX;
            }
            return NUMBER;
        }

        cout << "found invalid number declaration: " << word << endl;
        exit(-1);
    }
    cout << "encountered unrecognized symbol: " << word << endl;

    exit(-1);
}

string removeWhitespace(const string& word) {
    string result;
    const unordered_set noWhiteSpaceSymbols = {'(', ')', '{', '}', ';', ',', '[', ']', '='};
    const unordered_set skipSymbols = {' ', '\n', '\r', '\t'};

    constexpr char whitespace = ' ';
    result = word[0];
    bool space = skipSymbols.count(word[0]);

    for (int i = 1; i < word.size(); i++) {
        char cur = word[i];

        if (space) {
            if (!skipSymbols.count(cur)) {
                space = false;

                if (noWhiteSpaceSymbols.count(cur)) {
                    result += whitespace;
                    result += cur;
                    result += whitespace;
                }
                else {
                    result += whitespace;
                    result += cur;
                }
            }
        }
        else {
            if (skipSymbols.count(cur)) {
                space = true;
            }
            else {
                if (noWhiteSpaceSymbols.count(cur)) {
                    result += whitespace;
                    result += cur;
                    result += whitespace;
                }
                else {
                    result += cur;
                }
            }
        }
    }

    replaceAll(result, string(2,whitespace),string(1,whitespace));

    if (result[0] == whitespace) {
        result = result.substr(1,result.length()-1);
    }



    return result;
}

void replaceAll(string& result, const string& from, const string& to) {
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();  // Verhindert Endlosschleife
    }
}
