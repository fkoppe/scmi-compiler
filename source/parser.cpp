
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


std::shared_ptr<ASTNode> Parser::parseExpression() {
    return parseOrExpression(); // Logical OR is the top-level expression.
}

std::shared_ptr<ASTNode> Parser::parseOrExpression() {
    std::shared_ptr<ASTNode> left = parseAndExpression(); // `&&` has higher precedence than `||`.

    while (match(TokenType::OR)) {
        expect(TokenType::OR, "Expected '||' but found only '|'");
        left = std::make_shared<LogicalNode>(LogicalType::OR, left, parseAndExpression());
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseAndExpression() {
    std::shared_ptr<ASTNode> left = parseComparisonExpression(); // Comparison before AND.

    while (match(TokenType::AND)) {
        expect(TokenType::AND, "Expected '&&' but found only '&'");
        left = std::make_shared<LogicalNode>(LogicalType::AND, left, parseComparisonExpression());
    }

    return left;
}

// Comparison expressions (`==`, `!=`, `<`, `>`, `<=`, `>=`) go here.
std::shared_ptr<ASTNode> Parser::parseComparisonExpression() {
    std::shared_ptr<ASTNode> left = parseAdditiveExpression(); // Additions/subtractions are done first.

    while (true) {
        if (match(TokenType::ASSIGN)) {
            if (match(TokenType::ASSIGN)) { // `==`
                left = std::make_shared<LogicalNode>(LogicalType::EQUAL, left, parseAdditiveExpression());
            } else {
                std::cerr << "Parse Error: Expected '==' but found only '='\n";
                exit(1);
            }
        }
        else if (match(TokenType::NOT)) {
            expect(TokenType::ASSIGN, "Expected '!=' but found only '!'");
            left = std::make_shared<LogicalNode>(LogicalType::NOT_EQUAL, left, parseAdditiveExpression());
        }
        else if (match(TokenType::LESS)) {
            if (match(TokenType::ASSIGN)) { // `<=`
                left = std::make_shared<LogicalNode>(LogicalType::LESS_EQUAL, left, parseAdditiveExpression());
            } else {
                left = std::make_shared<LogicalNode>(LogicalType::LESS_THAN, left, parseAdditiveExpression());
            }
        }
        else if (match(TokenType::GREATER)) {
            if (match(TokenType::ASSIGN)) { // `>=`
                left = std::make_shared<LogicalNode>(LogicalType::GREATER_EQUAL, left, parseAdditiveExpression());
            } else {
                left = std::make_shared<LogicalNode>(LogicalType::GREATER_THAN, left, parseAdditiveExpression());
            }
        }
        else {
            break; // Exit loop if no comparison operators are found.
        }
    }

    return left;
}

// Addition and Subtraction (`+` and `-`) have lower precedence than Multiplication.
std::shared_ptr<ASTNode> Parser::parseAdditiveExpression() {
    std::shared_ptr<ASTNode> left = parseMultiplicativeExpression(); // First parse `*`, `/`, `%`.

    while (true) {
        if (match(TokenType::ADD)) { // `+`
            left = std::make_shared<ArithmeticNode>(ArithmeticType::ADD, left, parseMultiplicativeExpression());
        }
        else if (match(TokenType::SUB)) { // `-`
            left = std::make_shared<ArithmeticNode>(ArithmeticType::SUBTRACT, left, parseMultiplicativeExpression());
        }
        else {
            break;
        }
    }

    return left;
}

// Multiplication, Division, and Modulo (`*`, `/`, `%`) have highest precedence.
std::shared_ptr<ASTNode> Parser::parseMultiplicativeExpression() {
    std::shared_ptr<ASTNode> left = parseUnaryExpression(); // First handle `!x` or numbers.

    while (true) {
        if (match(TokenType::MULT)) { // `*`
            left = std::make_shared<ArithmeticNode>(ArithmeticType::MULTIPLY, left, parseUnaryExpression());
        }
        else if (match(TokenType::DIV)) { // `/`
            left = std::make_shared<ArithmeticNode>(ArithmeticType::DIVIDE, left, parseUnaryExpression());
        }
        else if (match(TokenType::MOD)) { // `%`
            left = std::make_shared<ArithmeticNode>(ArithmeticType::MODULO, left, parseUnaryExpression());
        }
        else {
            break;
        }
    }

    return left;
}

// Handles numbers, variables, parentheses, and negation.
std::shared_ptr<ASTNode> Parser::parseUnaryExpression() {
    if (match(TokenType::NOT)) { // Logical NOT (`!x`).
        return std::make_shared<LogicalNotNode>(parseUnaryExpression());
    }
    else if (match(TokenType::SUB)) { // Unary minus (`-x`).
        return std::make_shared<ArithmeticNode>(ArithmeticType::SUBTRACT,
                                                std::make_shared<NumberNode>(0), parseUnaryExpression());
    }

    return parsePrimaryExpression();
}

// Handles numbers, identifiers, functioncall expr, and parentheses.
std::shared_ptr<ASTNode> Parser::parsePrimaryExpression() {
    if (match(TokenType::NUMBER)) {
        return std::make_shared<NumberNode>(std::stoi(tokens[current - 1].raw));
    }
    else if (peek().type == TokenType::IDENTIFIER) {
        // If the next token is '(', it's a function call.
        if (peek2().type == TokenType::L_PAREN) {
            return parseFunctionCall();
        } else {
            std::string name = tokens[current].raw;
            advance();
            return std::make_shared<IdentifierNode>(name);
        }
    }
    else if (match(TokenType::L_PAREN)) { // Handling `(expression)`
        std::shared_ptr<ASTNode> expr = parseExpression();
        expect(TokenType::R_PAREN, "Expected closing ')'");
        return expr;
    }
    else {
        std::cerr << "Parse Error: Invalid expression: "
                  << tokens[current - 1].getTypeName()
                  << " '" << tokens[current - 1].raw << "' "
                  << tokens[current - 1].where() << "\n";
        exit(1);
    }

    return nullptr; // Should never reach this.
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

    if (peek().type == TokenType::KEYWORD && peek2().type == TokenType::L_BRACK && peek3().type == TokenType::R_BRACK) {
        if (peek().keyword != KeywordType::TYPE) throw runtime_error("Expected keyword type");

        string arrayTypeName = tokens[current].raw;
        advance();
        advance();
        advance();

        string arrayName = tokens[current].raw;
        expect(TokenType::IDENTIFIER, "Expected identifier in array assignment");

        expect(TokenType::ASSIGN, "Expected '=' in array assignment");


        //shared_ptr<ASTNode> arrayNode = make_shared<ArrayNode>()

        vector<shared_ptr<ASTNode>> arrayValues;
        int32_t size = -1;
        if (peek().type == TokenType::L_BRACE) {
            advance();
            // Falls Argumente vorhanden sind
            if (!match(TokenType::R_BRACE)) {
                arrayValues.push_back(parseExpression());
                while (match(TokenType::COMMA)) {
                    arrayValues.push_back(parseExpression());
                }
                expect(TokenType::R_BRACE, "Expected closing '}'");
            }
        }
        else if (peek().type == TokenType::KEYWORD) {
            advance();
            expect(TokenType::L_BRACK, "Expected [");

            size = stoi(tokens[current].raw);
            expect(TokenType::NUMBER, "Expected number in array assignment");
            expect(TokenType::R_BRACK, "Expected ]");

        }
        else {
            throw runtime_error("Expected '{' or KEYWORD in array assignment");
        }
        expect(TokenType::SEMICOLON, "Expected ';' after array assignment");
        return make_shared<ArrayNode>(convertStringToType(arrayTypeName), size, arrayValues, arrayName);
    }





    //plain procedure call
    if(peek().type == TokenType::IDENTIFIER && peek2().type == TokenType::L_PAREN) {
        auto functionCall = parseFunctionCall();
        expect(TokenType::SEMICOLON, "Expected ';' after function call");
        return functionCall;
    }


    //Handle function definition
    //keyword identifier ( keyword identifier , keyword identifier ) { ... }
    if (peek().type == TokenType::KEYWORD && peek().keyword == KeywordType::TYPE && peek2().type == TokenType::IDENTIFIER && peek3().type == TokenType::L_PAREN) {
        string returnTypeName = tokens[current].raw;
        advance();

        expect(TokenType::IDENTIFIER, "Expected function name after return type");
        string functionName = tokens[current - 1].raw;

        expect(TokenType::L_PAREN, "Expected '(' after function name");

        vector<pair<Type, string>> parameters;
        if (!match(TokenType::R_PAREN)) {
            do {
                expect(TokenType::KEYWORD, "Expected parameter type");
                string paramType = tokens[current - 1].raw;

                expect(TokenType::IDENTIFIER, "Expected parameter name");
                string paramName = tokens[current - 1].raw;
                parameters.emplace_back(convertStringToType(paramType), paramName);
            } while (match(TokenType::COMMA));

            expect(TokenType::R_PAREN, "Expected ')' at the end of parameter list");
        }

        expect(TokenType::L_BRACE, "Expected '{' to start function body");

        vector<shared_ptr<ASTNode>> body;
        while (!match(TokenType::R_BRACE)) {
            body.push_back(parseStatement()); // Parse function body statements
        }

        return make_shared<FunctionDefinitionNode>(convertStringToType(returnTypeName), functionName, parameters, body);
    }

    //Handle variable declaration
    //keyword identifier = ... ;
    if (peek().type == TokenType::KEYWORD && peek2().type == TokenType::IDENTIFIER && peek3().type == TokenType::ASSIGN) {
        auto varType = tokens[current].raw;
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
        return make_shared<VariableDeclarationNode>(convertStringToType(varType), varName, value);
    }


    // Handle assignment: identifier = ... ;
    if (peek().type == TokenType::IDENTIFIER && peek2().type == TokenType::ASSIGN) {
        string identifier = tokens[current].raw;
        advance();

        expect(TokenType::ASSIGN, "Expected '=' in assignment");

        /*
        // Handle array declaration: keyword identifier [ number ] = { number, number, ... };
        if (peek().type == TokenType::KEYWORD) {
            string arrayType = tokens[current].raw;
            advance();

            expect(TokenType::L_BRACK, "Expected '[' for array size declaration");
            expect(TokenType::NUMBER, "Expected array size");
            int arraySize = stoi(tokens[current - 1].raw);
            expect(TokenType::R_BRACK, "Expected ']' after array size");

            expect(TokenType::IDENTIFIER, "Expected array name");
            string arrayName = tokens[current - 1].raw;

            expect(TokenType::ASSIGN, "Expected '=' for array initialization");
            expect(TokenType::L_BRACE, "Expected '{' for array initialization");

            vector<shared_ptr<ASTNode>> arrayValues;
            do {
                //arrayValues.push_back(parseriableDeclaration(int g)Expression());
            } while (match(TokenType::COMMA));

            expect(TokenType::R_BRACE, "Expected '}' to close array initialization");
            expect(TokenType::SEMICOLON, "Expected ';' after array declaration");

            return make_shared<ArrayDeclarationNode>(identifier, convertStringToType(arrayType), arraySize, arrayName, arrayValues);
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

        cerr << "Parse Error: Unexpected token in statement: " << peek().getTypeName() << " '" << peek().raw << "' "<< peek().where() << "\n";
        exit(1);
    }

    // Handle `if` statement
    if (peek().type == TokenType::KEYWORD && peek().keyword == KeywordType::IF) {
        advance(); // Consume `if`
        expect(TokenType::L_PAREN, "Expected '(' after 'if'");
        auto condition = parseExpression(); // Parse the condition
        expect(TokenType::R_PAREN, "Expected ')' after condition");

        expect(TokenType::L_BRACE, "Expected '{' to start 'if' block");
        std::vector<std::shared_ptr<ASTNode>> thenBlock;
        while (!match(TokenType::R_BRACE)) {
            thenBlock.push_back(parseStatement()); // Recursively parse statements inside `if`
        }

        std::vector<std::shared_ptr<ASTNode>> elseBlock;

        // Handle `else if`
        while (peek().type == TokenType::KEYWORD && peek().keyword == KeywordType::ELSE) {
            advance(); // Consume `else`

            if (peek().type == TokenType::KEYWORD && peek().keyword == KeywordType::IF) {
                //advance(); // Consume `if`

                // `else if` is treated as an `if` inside the `elseBlock`
                elseBlock.push_back(parseStatement());
                return std::make_shared<IfNode>(condition, thenBlock, elseBlock);
            }

            // Handle regular `else`
            expect(TokenType::L_BRACE, "Expected '{' to start 'else' block");
            while (!match(TokenType::R_BRACE)) {
                elseBlock.push_back(parseStatement()); // Parse `else` block
            }
            break; // `else` must be the last branch
        }

        return std::make_shared<IfNode>(condition, thenBlock, elseBlock);
    }

    //return, goto, continue, break
    if (peek().type == TokenType::KEYWORD) {
        advance();
        string name = tokens[current - 1].raw;

        if(peek().type == TokenType::IDENTIFIER || peek().type == TokenType::NUMBER || peek().type == TokenType::L_PAREN) {
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

    cerr << "Parse Error: Unexpected statement: " << peek().getTypeName() << " '" << peek().raw << "' " << peek().where() << "\n";
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

