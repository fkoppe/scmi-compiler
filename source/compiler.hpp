//
// Created by cedric on 21.02.25.
//

#ifndef COMPILER_HPP
#define COMPILER_HPP
#include <memory>
#include <string>
#include <vector>

#include "ast.h"

using namespace std;

string compile(vector<shared_ptr<ASTNode>> ast);

struct Variable {
    string name;
    string adress;
};

class Function {
    public:
        explicit Function(FunctionDefinitionNode functionNode);

        void addVariable(VariableDeclarationNode declaration_node);

    private:
        vector<Variable> variableList;

};

#endif //COMPILER_HPP
