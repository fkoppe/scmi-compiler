//
// Created by cedric on 21.02.25.
//

#ifndef COMPILER_HPP
#define COMPILER_HPP
#include <memory>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

#include "ast.h"
#include "analyzer.hpp"

using namespace std;

/*
TODO:
- recursion analyze logicalExpression tree -> write in array✅
- figure out cmp and jump way to set comparsionNode✅
- output code with array to generate logicalExpression with stack operations✅
- change vector<LogicalExpression> to add function calls✅
*/

string compile(const vector<shared_ptr<ASTNode>>&, const vector<FunctionDescr>&, const unordered_map<string, unordered_map<string, Type>>&);
static void generateMallocFunction();


struct LocalVariable {
    Type type;
    string address;
};

using OperationUnion = variant<LogicalType, ArithmeticType>;

struct MathExpression {
    LocalVariable expression_L;
    LocalVariable expression_R;
    OperationUnion op;
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
        int jumpLabelNum;
        int registerNum;

        void generateNodes(const vector<shared_ptr<ASTNode>>&);
        FunctionDescr findFunctionDescr(const string&);
        void generateFunctionCall(const shared_ptr<FunctionCallNode>&, const FunctionDescr&);
        void generateAssignment(const LocalVariable& assign_variable, int index, const shared_ptr<ASTNode>& node_expression);
        void generateAssignment(const LocalVariable& assign_variable, const shared_ptr<ASTNode>& node_expression);
        int addVariables(const unordered_map<string, Type>&);
        void generateOutput(const shared_ptr<FunctionCallNode>&);
        void generateShift(const Type& from, const LocalVariable& to);
        static string getCompareJump(const LogicalType&);
        string getNextJumpLabel();
        LocalVariable getMathExpression(const shared_ptr<ASTNode>&, vector<MathExpression>&);
        void generateLogicalExpression(const MathExpression&);
        void generateArithmeticExpression(const MathExpression&, const Type& expected_type);
        void generateArithmeticOperation(ArithmeticType,Type);
        void malloc(int size, const string& assignment);
        void generateArrayIndexAssignment(const LocalVariable& array, int index);
        string getNextRegister();
        void clearRegisterNum();
        string generateArrayIndex(const LocalVariable& local_variable, int index);
        string getVariableAddress(const LocalVariable& local_variable, int index);
};

#endif //COMPILER_HPP
