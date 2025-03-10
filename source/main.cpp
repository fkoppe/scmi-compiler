#include <fstream>
#include <iostream>

#include "generator.hpp"
#include "Keyword.hpp"
#include "token.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "analyzer.hpp"
#include "rewriter.hpp"

void writeFile(string output, string filename);

int main(int argc, char* argv[]) {
    //auto tokens = lexString(readFile("../code.sc"));
    //Beck
    //string path = "../";
    //Koppe
    // string path = "../../";

    /*if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }*/

    string inputFile = argv[1];
    //string inputFile = "../../code.sc";

    string outputFile;
    string stdlib;
    if (argc == 3) {
        outputFile = argv[2];
        stdlib = argv[3];
    }
    else {
        stdlib = "./stdlib.sc";
        outputFile = "./output.mi";
    }


    Lexer lexer;
    //vector<Token> tokens = lexer.lexText("  void main(int y,int z){ int x = 5; int a=3;  int y = ggt(); y = ggt(); y = y; y = 1; ggt(); ggt(); return 0; return ggt(); return; if(a) { b = 0; }  }");

    std::string file_data = readFile(inputFile);
    std::string std_data = readFile(stdlib);

    file_data.append(std_data);


    vector<Token> tokens = lexer.lexText(file_data);


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
        cout << "Rewritten:\n";
        for(auto root : ast) {
            rewriter.rewrite(root);
            root->print();
        }


        cout << "\n\nOptimize:\n";
        for(auto root : ast) {
            rewriter.optimize(root);
        }

        std::cout << "=========================\n";


        cout << "\n=== COMPILE Output ===\n";
        string output = compile(ast, analysis.first, analysis.second);
        cout << output << endl;
        writeFile(output, outputFile);
        cout << "======================\n";

    } catch (const exception& e) {
        cerr << e.what() << "\n";
    }

    return 0;
}

void writeFile(string output, string filename) {
    std::ofstream file(filename);

    if (file.is_open()) {
        file << output;  // Write the string to the file
        file.close();  // Close the file
        std::cout << "File written successfully.\n";
    } else {
        std::cerr << "Error opening file!\n";
    }
}
