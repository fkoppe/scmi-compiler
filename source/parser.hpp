#ifndef PARSER_H
#define PARSER_H

#include "ast.h"  // AST-Knoten einbinden
#include <vector>
#include <memory>
#include "tokens.hpp"

// Parser-Klasse
class Parser {
private:
    std::vector<Token> tokens;
    size_t current;

    Token peek();
    Token advance();
    bool match(TokenType expected);
    void expect(TokenType expected, const std::string& errorMessage);

public:
    Parser(std::vector<Token> tokens);

    // Neue Rückgabewerte: AST-Knoten
    std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<ASTNode> parseFunctionCall();
    std::shared_ptr<ASTNode> parseStatement();

    std::vector<std::shared_ptr<ASTNode>> parse(); // Neuer Haupt-Parser
};

#endif // PARSER_H
