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
#include "analyzer.hpp"

using namespace std;

string compile(const vector<shared_ptr<ASTNode>>& ast);

const unordered_set<string> FORBIDDEN_IDENTIFIER_NAMES = {
    "int","short","char","float","double","return","void"
};

struct LocalVariable {
    VariableType type;
    string address;
};

class Function {
    public:
        Function(const shared_ptr<FunctionDefinitionNode>&, const unordered_map<string, VariableType>&, const vector<FunctionDescr>&);

        void addLocalVariable(const VariableDeclarationNode &declaration_node);

        string getOutput();

    private:
        unordered_map<string, LocalVariable> localVariableMap;
        vector<FunctionDescr> function_descrs;
        string output;
        int localVariablePointerOffset;
        int paramaterPointerOffset;
        LocalVariable findVariable(const string& name);
        FunctionDescr findFunctionDescr(const string& name);
        string getFunctionCall(const shared_ptr<FunctionCallNode>& function_call_node, const FunctionDescr& function_call_type);
        string getAssigment(const LocalVariable& assign_variable, const shared_ptr<ASTNode>& node_expression);



};

int getSize(const string& type);
string getMiType(const string& type);


#endif //COMPILER_HPP
