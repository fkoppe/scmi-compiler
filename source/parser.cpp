
#include "parser.hpp"

#include "ast.h"
#include <iostream>
#include <memory>

// Constructor
Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), current(0) {}

// Get the current token without consuming it
Token Parser::peek() {
    return (current < tokens.size()) ? tokens[current] : Token{TokenType::END_OF_FILE, ""};
}

Token Parser::peek2() {
    return (current < tokens.size() - 1) ? tokens[current + 1] : Token{TokenType::END_OF_FILE, ""};
}

Token Parser::peek3() {
    return (current < tokens.size() - 2) ? tokens[current + 2] : Token{TokenType::END_OF_FILE, ""};
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
        std::cerr << "Parse Error: " << errorMessage << " but found '" << peek().value << "' instead\n";
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
    //plain procedure call
    if(peek().type == TokenType::IDENTIFIER && peek2().type == TokenType::L_PAREN) {
        auto functionCall = parseFunctionCall();
        expect(TokenType::SEMICOLON, "Expected ';' after function call");
        return functionCall;
    }

    // Handle function definition: keyword identifier ( keyword identifier , keyword identifier ) { ... }
    if (peek().type == TokenType::KEYWORD && peek3().type == TokenType::L_PAREN) {
        std::string returnType = tokens[current].value;
        advance(); // Consume keyword

        expect(TokenType::IDENTIFIER, "Expected function name after return type");
        std::string functionName = tokens[current - 1].value;

        expect(TokenType::L_PAREN, "Expected '(' after function name");

        std::vector<std::pair<std::string, std::string>> parameters;
        if (!match(TokenType::R_PAREN)) {
            do {
                expect(TokenType::KEYWORD, "Expected parameter type");
                std::string paramType = tokens[current - 1].value;

                expect(TokenType::IDENTIFIER, "Expected parameter name");
                std::string paramName = tokens[current - 1].value;

                parameters.emplace_back(paramType, paramName);
            } while (match(TokenType::COMMA));

            expect(TokenType::R_PAREN, "Expected ')' at the end of parameter list");
        }

        expect(TokenType::L_BRACE, "Expected '{' to start function body");

        std::vector<std::shared_ptr<ASTNode>> body;
        while (!match(TokenType::R_BRACE)) {
            body.push_back(parseStatement()); // Parse function body statements
        }

        return std::make_shared<FunctionDefinitionNode>(returnType, functionName, parameters, body);
    }


    // Handle variable declaration: keyword identifier = ... ;
    if (peek().type == TokenType::KEYWORD && peek3().type == TokenType::ASSIGN) {
        std::string varType = tokens[current].value;
        advance();

        expect(TokenType::IDENTIFIER, "Expected variable name after type");
        std::string varName = tokens[current - 1].value;

        expect(TokenType::ASSIGN, "Expected '=' in variable declaration");

        std::shared_ptr<ASTNode> value;

        //function call: id()
        if(peek2().type == TokenType::L_PAREN) {
            value = parseFunctionCall();
        } else {
            //expression: exp
            value = parseExpression();
        }

        expect(TokenType::SEMICOLON, "Expected ';' at the end of declaration");
        return std::make_shared<VariableDeclarationNode>(varType, varName, value);
    }


    // Handle assignment: identifier = ... ;
    if (peek().type == TokenType::IDENTIFIER) {
        std::string identifier = tokens[current].value;
        advance();

        expect(TokenType::ASSIGN, "Expected '=' in assignment");

        /*
        // Handle array declaration: identifier keyword [ number ] identifier = { number, number, ... };
        if (peek().type == TokenType::KEYWORD) {
            std::string arrayType = tokens[current].value;
            advance();

            expect(TokenType::L_BRACK, "Expected '[' for array size declaration");
            expect(TokenType::NUMBER, "Expected array size");
            int arraySize = std::stoi(tokens[current - 1].value);
            expect(TokenType::R_BRACK, "Expected ']' after array size");

            expect(TokenType::IDENTIFIER, "Expected array name");
            std::string arrayName = tokens[current - 1].value;

            expect(TokenType::ASSIGN, "Expected '=' for array initialization");
            expect(TokenType::L_BRACE, "Expected '{' for array initialization");

            std::vector<std::shared_ptr<ASTNode>> arrayValues;
            do {
                //arrayValues.push_back(parseriableDeclaration(int g)Expression());
            } while (match(TokenType::COMMA));

            expect(TokenType::R_BRACE, "Expected '}' to close array initialization");
            expect(TokenType::SEMICOLON, "Expected ';' after array declaration");

            return std::make_shared<ArrayDeclarationNode>(identifier, arrayType, arraySize, arrayName, arrayValues);
        }
        */

        /*
        // Handle assignment: identifier [ number ] = number ;
        if (match(TokenType::L_BRACK)) {
            auto index = parseExpression();
            expect(TokenType::R_BRACK, "Expected ']' after array index");

            expect(TokenType::ASSIGN, "Expected '=' for array element assignment");
            auto value = parseExpression();

            expect(TokenType::SEMICOLON, "Expected ';' at the end of array assignment");
            return std::make_shared<ArrayAssignmentNode>(std::make_shared<IdentifierNode>(identifier), index, value);
        }
        */

        //function call
        if (peek2().type == TokenType::L_PAREN) {
            std::string funcName = tokens[current - 1].value;
            auto value = parseFunctionCall();
            expect(TokenType::SEMICOLON, "Expected ';' after function call");
            return std::make_shared<AssignmentNode>(std::make_shared<IdentifierNode>(identifier), value);
        }

        // Handle normal assignment
        if (peek().type == TokenType::IDENTIFIER || peek().type == TokenType::NUMBER || peek().type == TokenType::NUMBER_HEX) {
            auto expr = parseExpression();
            expect(TokenType::SEMICOLON, "Expected ';' at the end of assignment");
            return std::make_shared<AssignmentNode>(std::make_shared<IdentifierNode>(identifier), expr);
        }

        std::cerr << "Parse Error: Unexpected token in statement (" << tokenTypeName(peek().type) << ": " << peek().value << ")\n";
        exit(1);
    }

    //return, goto, continue, break
    if (peek().type == TokenType::CONTROL) {
        advance();
        std::string name = tokens[current - 1].value;

        if(peek().type == TokenType::IDENTIFIER || peek().type == TokenType::NUMBER || peek().type == TokenType::NUMBER_HEX) {
            if(peek().type == TokenType::IDENTIFIER && peek2().type == TokenType::L_PAREN) {
                auto st = parseStatement();

                return std::make_shared<ReturnValueNode>(st);
            }

            auto expr = parseExpression();
            std::string expression = tokens[current - 1].value;
            expect(TokenType::SEMICOLON, "Expected ';' at the end of statement");

            return std::make_shared<ReturnValueNode>(expr);
        } else {
            expect(TokenType::SEMICOLON, "Expected ';' at the end of statement");
            return std::make_shared<ReturnNode>();
        }
    }

    std::cerr << "Parse Error: Unexpected statement (" << tokenTypeName(peek().type) << ": " << peek().value << ")\n";
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

