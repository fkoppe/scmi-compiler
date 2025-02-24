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

struct VariableType {
    string name;
    int size;
};

struct Variable {
    string name;
    string address;
    VariableType type;
};

struct FunctionDescr {
    string name;
    VariableType type;
};


const unordered_set<string> FORBIDDEN_IDENTIFIER_NAMES = {
    "int","short","char","float","double","return","void"
};

class Function {
    public:
        explicit Function(const shared_ptr<FunctionDefinitionNode>& functionNode, const vector<FunctionDescr> &functionList);

        void addLocalVariable(const VariableDeclarationNode &declaration_node);
        void addInputVariable(const pair<string,string>& parameter);

        string getOutput();
        FunctionDescr getDescr();


    private:
        vector<Variable> variableList;
        vector<FunctionDescr> functionList;
        FunctionDescr functionDescr;
        string output;
        int localVariablePointerOffset;
        int paramaterPointerOffset;
        Variable findVariable(const string& name);
        FunctionDescr findFunctionDescr(const string& name);
        string getFunctionCall(const shared_ptr<FunctionCallNode>& function_call_node, const FunctionDescr& function_call_type);
        string getAssigment(const Variable& assign_variable, const shared_ptr<ASTNode>& node_expression);



};

void checkForbiddenIdentifier(const string& name);
int getSize(const string& type);
string getMiType(const string& type);


#endif //COMPILER_HPP
