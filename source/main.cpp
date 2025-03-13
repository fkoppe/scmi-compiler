#include <fstream>
#include <iostream>

#include "generator.hpp"
#include "Keyword.hpp"
#include "token.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "analyzer.hpp"
#include "rewriter.hpp"

//TODO:
//handle expression (funcCall) in mathExpression


void writeFile(string output, string filename, bool log);

int main(int argc, char* argv[]) {
    bool log = true;

    string inputFile = argv[1];
    string outputFile;
    string stdlib;
    if (argc == 4) {
        outputFile = argv[2];
        stdlib = argv[3];
    }
    else {
        stdlib = "./stdlib.sc";
        outputFile = "./output.mi";
    }

    try {
        Lexer lexer;
        //vector<Token> tokens = lexer.lexText("  void main(int y,int z){ int x = 5; int a=3;  int y = ggt(); y = ggt(); y = y; y = 1; ggt(); ggt(); return 0; return ggt(); return; if(a) { b = 0; }  }");

        std::string file_data = readFile(inputFile);
        file_data.append("\n");

        vector<Token> tokens = lexer.lexText(file_data, log);

        if (log) cout << "\n=== LEXER Output ===\n";
        printToken(tokens);
        if (log) cout << "====================\n";

        Parser parser = Parser(tokens, log);
        auto ast = parser.parse();

        if (log) {
            cout << "\n=== AST Output ===\n";
            for (const auto& node : ast) {
                node->print();
            }
            cout << "==================\n";
        }

        Lexer std_lexer;
        std::string std_data = readFile(stdlib);
        std_data.append("\n");
        vector<Token> std_tokens = std_lexer.lexText(std_data, false);
        Parser std_parser = Parser(std_tokens, false);
        auto std_ast = std_parser.parse();

        if (log) std::cout << "\n=== DEBUG ===\n";
        for(auto node : std_ast) {
            ast.push_back(node);
            node->print();
        }

        // Run semantic analysis
        if (log) std::cout << "\n=== Running Semantic Analysis ===\n";

        auto analysis = analyze(ast);

        if (log) cout << "Semantic analysis successful!\n";
        if (log) std::cout << "=================================\n";


        if (log) std::cout << "\n=== Running Rewriter ===\n";

        Rewriter rewriter;
        if (log) cout << "Rewritten:\n";
        for(auto root : ast) {
            rewriter.rewrite(root);
            if (log) root->print();
        }


        if (log) cout << "\n\nOptimize:\n";
        for(auto root : ast) {
            if (log) rewriter.optimize(root);
        }

        if (log) std::cout << "=========================\n";


        if (log) cout << "\n=== COMPILE Output ===\n";
        string output = compile(ast, analysis.first, analysis.second);
        if (log) cout << output << endl;
        writeFile(output, outputFile, log);
        if (log) cout << "======================\n";

    } catch (const exception& e) {
        cerr << e.what() << "\n";
    }

    return 0;
}

void writeFile(string output, string filename, bool log) {
    std::ofstream file(filename);

    if (file.is_open()) {
        file << output;  // Write the string to the file
        file.close();  // Close the file
        if (log) std::cout << "File written successfully.\n";
    } else {
        throw runtime_error("Error opening file!");
    }
}
