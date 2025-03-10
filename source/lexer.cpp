//
// Created by cedric on 20.02.25.
//

#include "lexer.hpp"

#include <fstream>
#include <string>
#include <iostream>

#include "token.hpp"

void Lexer::processWord() {
    TokenType type = getToken(word);

    if(type == TokenType::SPECIAL) {
        if(word == "false") {
            Token one = TokenType::NUMBER;
            one.line = line;
            one.num = num;
            one.number = NumberType::DECIMAL;
            one.raw = "1";

            result.push_back(one);
        }
        if(word == "true") {
            Token zero = TokenType::NUMBER;
            zero.line = line;
            zero.num = num;
            zero.number = NumberType::DECIMAL;
            zero.raw = "0";

            result.push_back(zero);
        }
        if(word == "string") {
            Token l = TokenType::L_BRACK;
            l.line = line;
            l.num = num;
            l.raw = "[";

            Token r = TokenType::R_BRACK;
            r.line = line;
            r.num = num;
            r.raw = "]";

            Token token = TokenType::KEYWORD;
            token.line = line;
            token.num = num;
            token.raw = "char";
            token.keyword = toKeywordType(word);

            result.push_back(token);
            result.push_back(l);
            result.push_back(r);
        }

        cout << word << " ";
        word.clear();

        return;
    }

    Token token = Token(type);
    token.line = line;
    token.num = num;
    token.raw = word;

    if(type == TokenType::KEYWORD) {
        token.keyword = toKeywordType(word);
    }

    cout << word << " ";
    result.push_back(token);

    word.clear();
}

vector<Token> Lexer::lexText(const string& text) {
    bool started_word = false;
    bool skipping_line = false;

    const unordered_set stopSymbols = {'(', ')', '{', '}', '[', ']', ';', ',', '=', '<', '>', '!', '&', '|', '\n', '\t', ' ', '+', '-', '*', '/', '%'};
    const unordered_set skipSymbols = {'\r', '\000'};

    cout << "\nLexing input..." << endl;

    for (size_t i = 0; i < text.size(); i++) {
        const char character = text.at(i);

        if(skipping_line) {
            if('\n' == character) {
                skipping_line = false;
            }
            continue;
        }

        if('/' == character) {
            if(text.size() > i + 1) {
                if(text.at(i + 1) == '/') {
                    skipping_line = true;
                    line++;
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
                num += 3;
            } else if('\n' == character) {
                line++;
                num = 0;
            } else if(' ' == character) {

            } else if(stopSymbols.count(character)) {
                word.push_back(character);
                processWord();
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
            num += 3;
        } else if('\n' == character) {
            line++;
            num = 0;
        } else if(' ' == character) {

        } else {
            word.push_back(character);
            processWord();
        }
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
    if(SPECIAL_SET.count(word)) {
        return TokenType::SPECIAL;
    }

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
        case '<':
            return TokenType::LESS;
        case '>':
            return TokenType::GREATER;
        case '!':
            return TokenType::NOT;
        case '&':
            return TokenType::AND;
        case '|':
            return TokenType::OR;
        case '+':
            return TokenType::ADD;
        case '-':
            return TokenType::SUB;
        case '*':
            return TokenType::MULT;
        case '/':
            return TokenType::DIV;
        case '%':
            return TokenType::MOD;
        case ';':
            return TokenType::SEMICOLON;
        case ',':
            return TokenType::COMMA;
    };

    if(isalpha(word[0]) || word[0] == '@') {
        return TokenType::IDENTIFIER;
    }

    if(word[0] == '#') {
        return TokenType::LABEL;
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
