#include <iostream>

#include "compiler.hpp"
#include "tokens.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "semantic_analyzer.hpp"

int main() {
    auto tokens = lexString(readFile("../code.sc"));
    //auto tokens = lexString("void main(int y,int z){int x = 5; int a=3;  int y = ggt(); y = ggt(); y = y; y = 1; ggt(); ggt(); return 0; return; return ggt(); }");

    Token eof = { TokenType::END_OF_FILE, "" };
    tokens.push_back(eof);

    std::cout << "=== LEXER Output ===\n";
    printToken(tokens);

    try {
        Parser parser = Parser(tokens);
        std::vector<std::shared_ptr<ASTNode>> ast = parser.parse();

        std::cout << "\n\n=== AST Output ===\n";

        for (const auto& node : ast) {
            node->print();
        }



        // Run semantic analysis
        std::cout << "\n=== Running Semantic Analysis ===\n";
        SemanticAnalyzer analyzer;
        analyzer.analyze(ast);

        std::cout << "Semantic analysis successful!\n";


        std::cout << "\n\n=== COMPILE Output ===\n";

        cout << compile(ast) << std::endl;

    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }

    return 0;
}
