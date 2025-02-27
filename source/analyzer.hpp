#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include <unordered_map>
#include <string>
#include <limits>
#include <unordered_set>

/*
TODO:
- Move small type to big type ✅
- If statement:
-   condition ✅
-   then/else normal überprüfen + keine declearations ✅
- rework getVariableType✅
- check return✅
*/


using namespace std;

const unordered_set<string> FORBIDDEN_IDENTIFIER_NAMES = {
    "int","short","char","float","double","return","void"
};

const unordered_set<string> FORBIDDEN_SUBSTRING = {"__return__"};
const string OUTPUT_FUNCTION = "@output";

struct FunctionDescr {
    string name;
    Type type; //type
    vector<pair<string,Type>> params; //vector<type>
};

pair<vector<FunctionDescr>,unordered_map<string,unordered_map<string,Type>>> analyze(vector<shared_ptr<ASTNode>>& nodes);
void checkFunctionNames(const vector<FunctionDescr>& function_descrs);
void checkForbiddenIdentifier(const string& name);


class SemanticAnalyzer {

public:
    SemanticAnalyzer(const shared_ptr<FunctionDefinitionNode>& function_node, const vector<FunctionDescr>& function_descrs);

    unordered_map<string, Type> getVariableList();
    string getName();

private:
    const static int maxInt = std::numeric_limits<int32_t>::max();
    const static int minInt = std::numeric_limits<int32_t>::min();
    const static int maxShort = std::numeric_limits<uint16_t>::max();
    const static int minShort = 0;
    const static int maxChar = std::numeric_limits<uint8_t>::max();
    const static int minChar = 0;

    unordered_map<string, Type> variableList; // Stores (variable name -> type)
    unordered_map<string, bool> isConstant; // Stores (variable name -> const status)
    string name;
    shared_ptr<FunctionDefinitionNode> function_node;
    vector<FunctionDescr> function_descrs;
    bool checkReturn;

    Type getArithmeticType(const shared_ptr<ArithmeticNode>&, const Type&);
    Type getCastType(Type, Type);
    Type getVariableType(const shared_ptr<ASTNode>&, const Type&);
    FunctionDescr findFunctionDescr(const string&);
    Type findVariable(const string&);
    FunctionDescr checkFunctionCall(const shared_ptr<FunctionCallNode>& function_call_node);
    void checkIdentifierType(string, Type, string, Type);
    void checkIdentifier(const shared_ptr<IdentifierNode>&);
    void checkAssignment(const shared_ptr<AssignmentNode>&);
    void checkDeclaration(const shared_ptr<VariableDeclarationNode>&);
    void checkParams();
    void checkLogicalExpression(const shared_ptr<ASTNode>&);
    void checkNode(const shared_ptr<ASTNode>&, bool);
    void checkExpression(const shared_ptr<ASTNode>&);

};

#endif // SEMANTIC_ANALYZER_H
