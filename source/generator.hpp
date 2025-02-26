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

/*
TODO:
- logical expression
- If statement:
-   condition ✅
-   then/else normal überprüfen + keine declearations ✅
- rework getVariableType✅
- check return✅
*/

string compile(const vector<shared_ptr<ASTNode>>&, const vector<FunctionDescr>&, const unordered_map<string, unordered_map<string, Type>>&);

struct LocalVariable {
    Type type;
    string address;
};

class Function {
    public:
        Function(const shared_ptr<FunctionDefinitionNode>&, const unordered_map<string, Type>&, const vector<FunctionDescr>&);
        string getOutput();

    private:
        unordered_map<string, LocalVariable> localVariableMap;
        vector<FunctionDescr> function_descr_vector;
        FunctionDescr function_descr_own;
        string output;
        string functionName;
        string returnLabel;
        int localVariablePointerOffset;
        int paramaterPointerOffset;
        FunctionDescr findFunctionDescr(const string&);
        void generateFunctionCall(const shared_ptr<FunctionCallNode>&, const FunctionDescr&);
        void generateAssignment(const LocalVariable& assign_variable, const shared_ptr<ASTNode>& node_expression);
        int addVariables(const unordered_map<string, Type>&);

        void generateOutput(const shared_ptr<FunctionCallNode>&);




};

#endif //COMPILER_HPP
