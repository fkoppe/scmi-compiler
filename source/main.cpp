#include <iostream>

#include "generator.hpp"
#include "Keyword.hpp"
#include "token.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "analyzer.hpp"
#include "rewriter.hpp"

int main() {
    //auto tokens = lexString(readFile("../code.sc"));

    Lexer lexer;
    //vector<Token> tokens = lexer.lexText("  void main(int y,int z){ int x = 5; int a=3;  int y = ggt(); y = ggt(); y = y; y = 1; ggt(); ggt(); return 0; return ggt(); return; if(a) { b = 0; }  }");
    vector<Token> tokens = lexer.lexText(readFile("../../code.sc"));


    cout << "\n=== LEXER Output ===\n";
    printToken(tokens);
    cout << "====================\n";

    try {
        Parser parser = Parser(tokens);
        auto ast = parser.parse();

        cout << "\n=== AST Output ===\n";
        for (const auto& node : ast) {
            node->print();
        }
        cout << "==================\n";



        // Run semantic analysis
        std::cout << "\n=== Running Semantic Analysis ===\n";

        auto analysis = analyze(ast);

        cout << "Semantic analysis successful!\n";
        std::cout << "=================================\n";


        std::cout << "\n=== Running Rewriter ===\n";

        Rewriter rewriter;
        for(auto root : ast) {
            rewriter.optimize(root);
        }

        std::cout << "==========================\n";


        cout << "\n=== COMPILE Output ===\n";
        cout << compile(ast, analysis.first, analysis.second) << endl;
        cout << "======================\n";

    } catch (const exception& e) {
        cerr << e.what() << "\n";
    }

    return 0;
}
