//
// Created by cedric on 20.02.25.
//

#include "lexer.hpp"

#include <unordered_set>

string readFile(const string& path) {

}

vector<TokenType> lexString(const string& data) {
    const string whiteSpaceData = removeWhitespace(data);
    vector<string> splitData = split(whiteSpaceData, ' ');
    vector<TokenType> result;
    result.reserve(splitData.size());

    for (const string& x : splitData) {
        result.push_back(getToken(x));
    }
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
    /*switch (word) {
        case "int":
        case "short":
        case "char":
        case "void":
        case "float":
        case "double":
            return KEYWORD;
        case "{":
            return L_BRACK;
        case "}":
            return R_BRACK;
        case "(":
            return L_PAREN;
        case ")":
            return R_PAREN;
        case "=":
            return ASSIGN;
    }*/
    return ASSIGN;
}

string removeWhitespace(const string& word) {
    string result;
    const unordered_set noWhiteSpaceSymbols = {'(', ')', '{', '}', ';', ',', '[', ']', '='};
    const char whitespace = ' ';


    bool space = word[0] == ' ';

    for (int i = 1; i < word.size(); i++) {
        char cur = word[i];

        if (space) {
            if (cur != ' ') {
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
            if (cur == ' ') {
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

    if (result[0] == whitespace) {
        result = result.substr(1,result.length()-1);
    }

    replaceAll(result, string(2,whitespace),string(1,whitespace));

    return result;
}

void replaceAll(string& result, const string& from, const string& to) {
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();  // Verhindert Endlosschleife
    }
}
