#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include <unordered_map>
#include <string>
#include <unordered_set>

using namespace std;

/*TODO:
- check for same function names and one main function ✅
- check params and add to variableList ✅
- check forbidden identifier names ✅
- check for not declared variables ✅
- created variable list <name,type> ✅
- check for not declared function calls -> functionList
- check for same type when assigning (variable & function call)
- check for same type -> function params
- check for type max number when immediate value. Ex: char x = 256
*/

// const unordered_set<string> FORBIDDEN_IDENTIFIER_NAMES = {
//     "int","short","char","float","double","return","void"
// };

struct VariableType {
    string name;
};

struct S_Variable {
    string name;
    VariableType type;
};

struct S_FunctionDescr {
    string name;
    VariableType type; //type
    vector<pair<string,VariableType>> params; //vector<type>
};

pair<vector<S_FunctionDescr>,unordered_map<string,unordered_map<string,VariableType>>> analyze(vector<shared_ptr<ASTNode>>& nodes);
void checkFunctionNames(const vector<S_FunctionDescr>& function_descrs);
void checkForbiddenIdentifier(const string& name);


class SemanticAnalyzer {

public:
    SemanticAnalyzer(const shared_ptr<FunctionDefinitionNode>& function_node, const vector<S_FunctionDescr>& function_descrs);

    unordered_map<string, VariableType> getVariableList();
    string getName();



private:
    unordered_map<string, VariableType> variableList; // Stores (variable name -> type)
    unordered_map<string, bool> isConstant; // Stores (variable name -> const status)
    string name;
    vector<S_FunctionDescr> function_descrs;

    VariableType findVariable(const string&);
    void checkIdentifierType(IdentifierNode, IdentifierNode);
    void checkIdentifier(const shared_ptr<IdentifierNode>&);
    void checkAssignment(const shared_ptr<AssignmentNode>&);
    void checkDeclaration(const shared_ptr<VariableDeclarationNode>&);
    void checkParams();
};

#endif // SEMANTIC_ANALYZER_H
