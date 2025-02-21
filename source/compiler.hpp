//
// Created by cedric on 21.02.25.
//

#ifndef COMPILER_HPP
#define COMPILER_HPP
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "ast.h"

using namespace std;

string compile(const vector<shared_ptr<ASTNode>>& ast);

struct Variable {
    string name;
    string address;
};

const unordered_set<string> FORBIDDEN_IDENTIFIER_NAMES = {
    "int","short","char","float","double","return","void"
};

class Function {
    public:
        explicit Function(const shared_ptr<FunctionDefinitionNode>& functionNode);

        void addLocalVariable(const VariableDeclarationNode &declaration_node);
        void addInputVariable(const pair<string,string>& parameter);
        string getOutput();


    private:
        vector<Variable> variableList;
        string output;
        int localVariablePointerOffset;
        int paramaterPointerOffset;
        Variable findVariable(const string& name);

};


void checkForbiddenIdentifier(const string& name);
int getSize(const string& type);
string getMiType(const string& type);
#endif //COMPILER_HPP
