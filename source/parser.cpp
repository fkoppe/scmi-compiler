
#include "parser.hpp"

// Constructor
Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), current(0) {}

// Get the current token without consuming it
Token Parser::peek() {
    return (current < tokens.size()) ? tokens[current] : Token{TokenType::END_OF_FILE, ""};
}

// Consume the current token and move forward
Token Parser::advance() {
    return (current < tokens.size()) ? tokens[current++] : Token{TokenType::END_OF_FILE, ""};
}

// Match a token type and advance if it matches
bool Parser::match(TokenType expected) {
    if (peek().type == expected) {
        advance();
        return true;
    }
    return false;
}

// Expect a specific token, throw error if missing
void Parser::expect(TokenType expected, const std::string& errorMessage) {
    if (!match(expected)) {
        std::cerr << "Parse Error: " << errorMessage << "\n";
        exit(1);
    }
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    if (match(TokenType::NUMBER)) {
        return std::make_shared<NumberNode>(std::stoi(tokens[current - 1].value));
    }
    else if (match(TokenType::IDENTIFIER)) {
        std::shared_ptr<IdentifierNode> identifier = std::make_shared<IdentifierNode>(tokens[current - 1].value);
        if (match(TokenType::ASSIGN)) {
            auto expr = parseExpression();
            return std::make_shared<AssignmentNode>(identifier, expr);
        }
        return identifier;  // Falls es nur eine Variable ist
    }
    else if (match(TokenType::L_PAREN)) {
        auto expr = parseExpression();
        expect(TokenType::R_PAREN, "Expected closing ')'");
        return expr;
    }

    std::cerr << "Parse Error: Invalid expression\n";
    exit(1);
}


// Parse a function call
std::shared_ptr<ASTNode> Parser::parseFunctionCall() {
    expect(TokenType::IDENTIFIER, "Expected function name");
    std::string functionName = tokens[current - 1].value;
    expect(TokenType::L_PAREN, "Expected '(' after function name");

    auto functionCall = std::make_shared<FunctionCallNode>(functionName);

    // Falls Argumente vorhanden sind
    if (!match(TokenType::R_PAREN)) {
        functionCall->arguments.push_back(parseExpression());
        while (match(TokenType::COMMA)) {
            functionCall->arguments.push_back(parseExpression());
        }
        expect(TokenType::R_PAREN, "Expected closing ')'");
    }

    return functionCall;
}


// Parse a statement (expression followed by a semicolon)
std::shared_ptr<ASTNode> Parser::parseStatement() {
    if (peek().type == TokenType::IDENTIFIER) {
        std::string identifier = tokens[current].value;
        advance();

        if (match(TokenType::ASSIGN)) {
            auto expr = parseExpression();
            expect(TokenType::SEMICOLON, "Expected ';' at the end of assignment");
            return std::make_shared<AssignmentNode>(std::make_shared<IdentifierNode>(identifier), expr);
        }
        else if (match(TokenType::L_PAREN)) {
            auto functionCall = parseFunctionCall();
            expect(TokenType::SEMICOLON, "Expected ';' after function call");
            return functionCall;
        }
        else {
            std::cerr << "Parse Error: Unexpected token in statement\n";
            exit(1);
        }
    }

    std::cerr << "Parse Error: Unexpected statement\n";
    exit(1);
}


// Main parse function
std::vector<std::shared_ptr<ASTNode>> Parser::parse() {
    std::vector<std::shared_ptr<ASTNode>> ast;
    while (peek().type != TokenType::END_OF_FILE) {
        ast.push_back(parseStatement());
    }
    return ast;
}

