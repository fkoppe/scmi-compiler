#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include <unordered_map>
#include <string>

using namespace std;

class SemanticAnalyzer {

public:
    void analyze(vector<shared_ptr<ASTNode>>& ast);

private:
    unordered_map<string, string> symbolTable; // Stores (variable name -> type)
    unordered_map<string, bool> isConstant; // Stores (variable name -> const status)

    void analyzeNode(shared_ptr<ASTNode> node);
    void analyzeAssignment(shared_ptr<AssignmentNode> node);
    void analyzeVariableDeclaration(shared_ptr<VariableDeclarationNode> node);
    void analyzeFunctionDefinition(shared_ptr<FunctionDefinitionNode> node);
    void analyzeArrayAssignment(shared_ptr<ArrayAssignmentNode> node);
};

#endif // SEMANTIC_ANALYZER_H
