#include <iostream>

#include "compiler.hpp"
#include "tokens.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "semantic_analyzer.hpp"

int main() {
    //auto tokens = lexString(readFile("../code.sc"));

    Lexer lexer;
    vector<Token> tokens = lexer.lexText("  void main(int y,int z){ int x = 5; int a=3;  int y = ggt(); y = ggt(); y = y; y = 1; ggt(); ggt(); return 0; return ggt(); return;  }");

    cout << "\n=== LEXER Output ===\n";
    printToken(tokens);
    cout << "====================\n";

    try {
        Parser parser = Parser(tokens);
        auto ast = parser.parse();

        cout << "\n\n=== AST Output ===\n";
        for (const auto& node : ast) {
            node->print();
        }
        cout << "\n\n==================\n";



        // Run semantic analysis
        cout << "\n=== Running Semantic Analysis ===\n";
        SemanticAnalyzer analyzer;
        analyzer.analyze(ast);

        cout << "Semantic analysis successful!\n";


        cout << "\n\n=== COMPILE Output ===\n";
        cout << compile(ast) << endl;
        cout << "\n\n======================\n";

    } catch (const exception& e) {
        cerr << e.what() << "\n";
    }

    return 0;
}
