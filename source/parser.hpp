#ifndef PARSER_H
#define PARSER_H

#include "ast.h"  // AST-Knoten einbinden
#include <vector>
#include <memory>
#include "token.hpp"

using namespace std;

// Parser-Klasse
class Parser {
private:
    vector<Token> tokens;
    size_t current = 0;

    Token peek();
    Token peek2();
    Token peek3();
    Token advance();
    bool match(TokenType expected);
    void expect(TokenType expected, const string& errorMessage);

public:
    Parser(vector<Token> tokens);

    // Neue Rückgabewerte: AST-Knoten
    shared_ptr<ASTNode> parseExpression();
    shared_ptr<ASTNode> parseComparisonExpression();
    shared_ptr<ASTNode> parseUnaryExpression();
    shared_ptr<ASTNode> parseFunctionCall();
    shared_ptr<ASTNode> parseStatement();

    vector<shared_ptr<ASTNode>> parse(); // Neuer Haupt-Parser
};

#endif // PARSER_H
