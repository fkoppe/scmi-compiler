#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include <unordered_map>
#include <string>
#include <limits>
#include <unordered_set>

using namespace std;

/*TODO:
- check for same function names and one main function ✅
- check params and add to variableList ✅
- check forbidden identifier names ✅
- check for not declared variables ✅
- created variable list <name,type> ✅
- check for not declared function calls -> functionList ✅
- check for same type when assigning (variable & function call) ✅
- check for same type -> function params ✅
- check for type max number when immediate value. Ex: char x = 256 ✅
*/

// const unordered_set<string> FORBIDDEN_IDENTIFIER_NAMES = {
//     "int","short","char","float","double","return","void"
// };

struct VariableType {
    string name;
};

struct FunctionDescr {
    string name;
    VariableType type; //type
    vector<pair<string,VariableType>> params; //vector<type>
};

pair<vector<FunctionDescr>,unordered_map<string,unordered_map<string,VariableType>>> analyze(vector<shared_ptr<ASTNode>>& nodes);
void checkFunctionNames(const vector<FunctionDescr>& function_descrs);
void checkForbiddenIdentifier(const string& name);


class SemanticAnalyzer {

public:
    SemanticAnalyzer(const shared_ptr<FunctionDefinitionNode>& function_node, const vector<FunctionDescr>& function_descrs);

    unordered_map<string, VariableType> getVariableList();
    string getName();



private:
    const static int maxInt = std::numeric_limits<int32_t>::max();
    const static int minInt = std::numeric_limits<int32_t>::min();
    const static int maxShort = std::numeric_limits<uint16_t>::max();
    const static int minShort = 0;
    const static int maxChar = std::numeric_limits<uint8_t>::max();
    const static int minChar = 0;

    unordered_map<string, VariableType> variableList; // Stores (variable name -> type)
    unordered_map<string, bool> isConstant; // Stores (variable name -> const status)
    string name;
    vector<FunctionDescr> function_descrs;

    VariableType getVariableType(const shared_ptr<ASTNode>&, const VariableType&);
    FunctionDescr findFunctionDescr(const string&);
    VariableType findVariable(const string&);
    FunctionDescr checkFunctionCall(const shared_ptr<FunctionCallNode>& function_call_node);
    void checkIdentifierType(string, VariableType, string, VariableType);
    void checkIdentifier(const shared_ptr<IdentifierNode>&);
    void checkAssignment(const shared_ptr<AssignmentNode>&);
    void checkDeclaration(const shared_ptr<VariableDeclarationNode>&);
    void checkParams();
};

#endif // SEMANTIC_ANALYZER_H
