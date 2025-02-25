
#include "parser.hpp"

#include "ast.h"
#include <iostream>
#include <memory>

Parser::Parser(vector<Token> tokens_) {
    tokens = tokens_;
}

Token Parser::peek() {
    return (current < tokens.size()) ? tokens[current] : eof;
}

Token Parser::peek2() {
    return (current < tokens.size() - 1) ? tokens[current + 1] : eof;
}

Token Parser::peek3() {
    return (current < tokens.size() - 2) ? tokens[current + 2] : eof;
}

// Consume the current token and move forward
Token Parser::advance() {
    return (current < tokens.size()) ? tokens[current++] : eof;
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
void Parser::expect(TokenType expected, const string& errorMessage) {
    if (!match(expected)) {
        cerr << "Parse Error: " << errorMessage << " but found " << peek().getTypeName() << " '" << peek().raw << "' instead " << peek().where() << endl;
        exit(1);
    }
}

shared_ptr<ASTNode> Parser::parseExpression() {
    if (match(TokenType::NUMBER)) {
        return make_shared<NumberNode>(stoi(tokens[current - 1].raw));
    }
    else if (match(TokenType::IDENTIFIER)) {
        shared_ptr<IdentifierNode> identifier = make_shared<IdentifierNode>(tokens[current - 1].getTypeName());
        if (match(TokenType::ASSIGN)) {
            auto expr = parseExpression();
            return make_shared<AssignmentNode>(identifier, expr);
        }
        return identifier;  // Falls es nur eine Variable ist
    }
    else if (match(TokenType::L_PAREN)) {
        auto expr = parseExpression();
        expect(TokenType::R_PAREN, "Expected closing ')'");
        return expr;
    }

    cerr << "Parse Error: Invalid expression: " << tokens[current - 1].getTypeName() << " '" << tokens[current - 1].raw << "'" << endl;
    exit(1);
}


// Parse a function call
shared_ptr<ASTNode> Parser::parseFunctionCall() {
    expect(TokenType::IDENTIFIER, "Expected function name");
    string functionName = tokens[current - 1].raw;
    expect(TokenType::L_PAREN, "Expected '(' after function name");

    auto functionCall = make_shared<FunctionCallNode>(functionName);

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
shared_ptr<ASTNode> Parser::parseStatement() {

    //plain procedure call
    if(peek().type == TokenType::IDENTIFIER && peek2().type == TokenType::L_PAREN) {
        auto functionCall = parseFunctionCall();
        expect(TokenType::SEMICOLON, "Expected ';' after function call");
        return functionCall;
    }


    //Handle function definition
    //keyword identifier ( keyword identifier , keyword identifier ) { ... }
    if (peek().type == TokenType::KEYWORD && peek().keyword == KeywordType::TYPE && peek2().type == TokenType::IDENTIFIER && peek3().type == TokenType::L_PAREN) {
        string returnTypeName = tokens[current].getTypeName();
        advance();

        expect(TokenType::IDENTIFIER, "Expected function name after return type");
        string functionName = tokens[current - 1].raw;

        expect(TokenType::L_PAREN, "Expected '(' after function name");

        vector<pair<string, string>> parameters;
        if (!match(TokenType::R_PAREN)) {
            do {
                expect(TokenType::KEYWORD, "Expected parameter type");
                string paramType = tokens[current - 1].getTypeName();

                expect(TokenType::IDENTIFIER, "Expected parameter name");
                string paramName = tokens[current - 1].raw;

                parameters.emplace_back(paramType, paramName);
            } while (match(TokenType::COMMA));

            expect(TokenType::R_PAREN, "Expected ')' at the end of parameter list");
        }

        expect(TokenType::L_BRACE, "Expected '{' to start function body");

        vector<shared_ptr<ASTNode>> body;
        while (!match(TokenType::R_BRACE)) {
            body.push_back(parseStatement()); // Parse function body statements
        }

        return make_shared<FunctionDefinitionNode>(returnTypeName, functionName, parameters, body);
    }

    //Handle variable declaration
    //keyword identifier = ... ;
    if (peek().type == TokenType::KEYWORD && peek2().type == TokenType::IDENTIFIER && peek3().type == TokenType::ASSIGN) {
        auto varType = tokens[current].type;
        advance();

        expect(TokenType::IDENTIFIER, "Expected variable name after type");
        string varName = tokens[current - 1].raw;

        expect(TokenType::ASSIGN, "Expected '=' in variable declaration");

        shared_ptr<ASTNode> value;

        //function call: id()
        if(peek2().type == TokenType::L_PAREN) {
            value = parseFunctionCall();
        } else {
            value = parseExpression();
        }

        expect(TokenType::SEMICOLON, "Expected ';' at the end of declaration");
        return make_shared<VariableDeclarationNode>(toTypeType(varType), varName, value);
    }


    // Handle assignment: identifier = ... ;
    if (peek().type == TokenType::IDENTIFIER && peek2().type == TokenType::ASSIGN) {
        string identifier = tokens[current].raw;
        advance();

        expect(TokenType::ASSIGN, "Expected '=' in assignment");

        /*
        // Handle array declaration: identifier keyword [ number ] identifier = { number, number, ... };
        if (peek().type == TokenType::KEYWORD) {
            string arrayType = tokens[current].value;
            advance();

            expect(TokenType::L_BRACK, "Expected '[' for array size declaration");
            expect(TokenType::NUMBER, "Expected array size");
            int arraySize = stoi(tokens[current - 1].value);
            expect(TokenType::R_BRACK, "Expected ']' after array size");

            expect(TokenType::IDENTIFIER, "Expected array name");
            string arrayName = tokens[current - 1].value;

            expect(TokenType::ASSIGN, "Expected '=' for array initialization");
            expect(TokenType::L_BRACE, "Expected '{' for array initialization");

            vector<shared_ptr<ASTNode>> arrayValues;
            do {
                //arrayValues.push_back(parseriableDeclaration(int g)Expression());
            } while (match(TokenType::COMMA));

            expect(TokenType::R_BRACE, "Expected '}' to close array initialization");
            expect(TokenType::SEMICOLON, "Expected ';' after array declaration");

            return make_shared<ArrayDeclarationNode>(identifier, arrayType, arraySize, arrayName, arrayValues);
        }



        // Handle assignment: identifier [ number ] = number ;
        if (match(TokenType::L_BRACK)) {
            auto index = parseExpression();
            expect(TokenType::R_BRACK, "Expected ']' after array index");

            expect(TokenType::ASSIGN, "Expected '=' for array element assignment");
            auto value = parseExpression();

            expect(TokenType::SEMICOLON, "Expected ';' at the end of array assignment");
            return make_shared<ArrayAssignmentNode>(make_shared<IdentifierNode>(identifier), index, value);
        }
        */


        //function call
        if (peek2().type == TokenType::L_PAREN) {
            string funcName = tokens[current - 1].raw;
            auto value = parseFunctionCall();
            expect(TokenType::SEMICOLON, "Expected ';' after function call");
            return make_shared<AssignmentNode>(make_shared<IdentifierNode>(identifier), value);
        }

        // Handle normal assignment
        if (peek().type == TokenType::IDENTIFIER || peek().type == TokenType::NUMBER) {
            auto expr = parseExpression();
            expect(TokenType::SEMICOLON, "Expected ';' at the end of assignment");
            return make_shared<AssignmentNode>(make_shared<IdentifierNode>(identifier), expr);
        }

        cerr << "Parse Error: Unexpected token in statement: " << peek().getTypeName() << " '" << peek().raw << "'\n";
        exit(1);
    }

    //return, goto, continue, break
    if (peek().type == TokenType::KEYWORD) {
        advance();
        string name = tokens[current - 1].raw;

        if(peek().type == TokenType::IDENTIFIER || peek().type == TokenType::NUMBER) {
            if(peek().type == TokenType::IDENTIFIER && peek2().type == TokenType::L_PAREN) {
                auto st = parseStatement();

                return make_shared<ReturnValueNode>(st);
            }

            auto expr = parseExpression();
            string expression = tokens[current - 1].raw;
            expect(TokenType::SEMICOLON, "Expected ';' at the end of statement");

            return make_shared<ReturnValueNode>(expr);
        } else {
            expect(TokenType::SEMICOLON, "Expected ';' at the end of statement");
            return make_shared<ReturnNode>();
        }
    }

    cerr << "Parse Error: Unexpected statement: " << peek().getTypeName() << " '" << peek().raw << "'\n";
    exit(1);
}



// Main parse function
vector<shared_ptr<ASTNode>> Parser::parse() {
    cout << "\nParsing " << tokens.size() << " tokens..." << endl;

    vector<shared_ptr<ASTNode>> ast;

    while (peek().type != TokenType::END_OF_FILE) {
        ast.push_back(parseStatement());
    }

    cout << "AST size of " << ast.size() << " nodes" << endl;

    return ast;
}

