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

string compile(const vector<shared_ptr<ASTNode>>& ast);

struct Variable {
    string name;
    string address;
};

class Function {
    public:
        explicit Function(const FunctionDefinitionNode& functionNode);

        void addLocalVariable(const VariableDeclarationNode &declaration_node);


    private:
        vector<Variable> variableList;
        string output;
        int localVariablePointerOffset;
        Variable findVariable(const string& name);

};

int getSize(const string& type);
string getMiType(const string& type);
#endif //COMPILER_HPP
