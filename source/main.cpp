#include <iostream>

#include "tokens.hpp"
#include "lexer.hpp"
#include "parser.hpp"

int main() {
    auto tokens = lexString("  void main(){ int i = 0; }");
    //auto tokens = lexString("  void main(int y,int z){    int x = 0x5;long y=3;   const int[5  ] y = {1,  2, 3   , 4,  5};    y[4] = 4;    print(y); }    ");

    Token eof = { END_OF_FILE, "" };
    tokens.push_back(eof);

    printTokens(tokens);

    try {
        Parser parser = Parser(tokens);
        std::vector<std::shared_ptr<ASTNode>> ast = parser.parse();

        std::cout << "=== AST Output ===\n";
        for (const auto& node : ast) {
            node->print();
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }

    return 0;
}
