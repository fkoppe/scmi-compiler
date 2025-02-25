//
// Created by cedric on 20.02.25.
//

#include "lexer.hpp"

#include <fstream>
#include <string>
#include <iostream>

void Lexer::processWord() {
    TokenType type = getToken(word);

    Token token = Token(type);
    token.line = line;
    token.num = num;
    token.raw = word;

    if(type == TokenType::KEYWORD) {
        if(TYPE_SET.count(word)) {
            token.keyword = toKeywordType(word);
        }
    }

    cout << word << " ";
    result.push_back(token);
    word.clear();
}

vector<Token> Lexer::lexText(const string& text) {
    uint64_t line = 0;
    uint64_t num = 0;

    bool started_word = false;
    bool skipping_line = false;

    const unordered_set stopSymbols = {'(', ')', '{', '}', '[', ']', ';', ',', '=', '\n', '\t', ' '};
    const unordered_set skipSymbols = {'\r'};

    cout << "\nLexing input..." << endl;

    for (size_t i = 0; i < text.size(); i++) {
        const char character = text.at(i);

        if(skipping_line) {
            if('\n' == character) {
                skipping_line = false;
                continue;
            }
        }

        if('/' == character) {
            if(text.size() > i + 1) {
                if(text.at(i + 1) == '/') {
                    skipping_line = true;
                    continue;
                }
            }
        }

        num++;

        if(skipSymbols.count(character)) {
            continue;
        }

        if(!started_word) {
            if('\t' == character) {
                num += 4;
            } else if('\n' == character) {
                line++;
                num = 0;
            } else if(' ' == character) {
                num++;
            } else {
                started_word = true;
                word.push_back(character);
            }

            continue;
        }

        if(!stopSymbols.count(character)) {
            word.push_back(character);
            continue;
        }

        started_word = false;
        processWord();

        if('\t' == character) {
            num += 4;
        } else if('\n' == character) {
            line++;
            num = 0;
        } else if(' ' == character) {
            num++;
        } else {
            word.push_back(character);
            processWord();
        }
    }

    if(word.size() > 0) {
        processWord();
    }

    cout << "\nLexer reached EOF" << endl;

    return result;
}

string readFile(const string& path) {
    ifstream file(path, ios::ate);
    if (!file) {
        throw runtime_error("Could not open file: " + path);
    }
    const streamsize size = file.tellg();
    file.seekg(0);
    string content = string(size, '\0');
    file.read(content.data(), size);
    return content;
}

TokenType getToken(const string& word) {
    if(KEYWORD_SET.count(word)) {
        return TokenType::KEYWORD;
    }

    switch (word[0]) {
        case '{':
            return TokenType::L_BRACE;
        case '}':
            return TokenType::R_BRACE;
        case '(':
            return TokenType::L_PAREN;
        case ')':
            return TokenType::R_PAREN;
        case '[':
            return TokenType::L_BRACK;
        case ']':
            return TokenType::R_BRACK;
        case '=':
            return TokenType::ASSIGN;
        case ';':
            return TokenType::SEMICOLON;
        case ',':
            return TokenType::COMMA;
    };

    if(isalpha(word[0])) {
        return TokenType::IDENTIFIER;
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
            //if (hex) {
            //    return TokenType::NUMBER_HEX;
            //}
            return TokenType::NUMBER;
        }

        cout << "\nfound invalid number declaration: >" << word << "<" << endl;
        exit(-1);
    }
    cout << "\nencountered unrecognized symbol: >" << word << "<" << endl;

    exit(-1);
}
