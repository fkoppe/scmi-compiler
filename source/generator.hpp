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


string compile(const vector<shared_ptr<ASTNode>>&, const vector<FunctionDescr>&, const unordered_map<string, unordered_map<string, Type>>&);


struct LocalVariable {
    Type type;
    string address;
};

using OperationUnion = variant<LogicalType, ArithmeticType>;

struct MathExpression {
    shared_ptr<ASTNode> expression_L;
    shared_ptr<ASTNode> expression_R;
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
        const int ARRAY_DESCRIPTOR_SIZE = 4;

        void generateNodes(const vector<shared_ptr<ASTNode>>&);
        FunctionDescr findFunctionDescr(shared_ptr<FunctionCallNode>);
        FunctionDescr findFunctionDescr(shared_ptr<FunctionDefinitionNode>);

        void generateFunctionCall(const shared_ptr<FunctionCallNode>&, const FunctionDescr&);
        void generateAssignment(const LocalVariable& assign_variable, shared_ptr<ASTNode> index, const shared_ptr<ASTNode>& node_expression);
        void generateAssignment(const LocalVariable& assign_variable, const shared_ptr<ASTNode>& node_expression);
        int addVariables(const unordered_map<string, Type>&);
        void generateOutputFunction(const shared_ptr<FunctionCallNode>&);
        void generateShift(const Type& from, const LocalVariable& to);
        static string getCompareJump(const LogicalType&);
        string getNextJumpLabel();
        shared_ptr<ASTNode> getMathExpression(const shared_ptr<ASTNode>&, vector<MathExpression>&);
        void generateLogicalExpression(const MathExpression&);
        void generateArithmeticExpression(const MathExpression&, const Type& expected_type);
        void generateArithmeticOperation(ArithmeticType,Type);
        void malloc(int size, const string& assignment);
        string generateArrayIndex(const LocalVariable& local_variable, shared_ptr<ASTNode> index);

        string getNextRegister();
        void clearRegisterNum();
        string getVariableAddress(const LocalVariable& local_variable, shared_ptr<ASTNode> index);
        void generateMathExpression(const shared_ptr<ASTNode>&, Type);
        void generateSREF(shared_ptr<FunctionCallNode>);
        Type getType(shared_ptr<ASTNode>);
};

#endif //COMPILER_HPP
