#include <iostream>

#include "tokens.hpp"
#include "lexer.hpp"
#include "parser.hpp"

int main() {
    auto tokens = lexString("  void main(int y,int z){    int x = 5;long y=3;   const int[5  ] y = {1,  2, 3   , 4,  5};    y[4] = 4;    print(y); }    ");

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
