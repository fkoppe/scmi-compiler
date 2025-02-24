#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include <unordered_map>
#include <string>
#include <iostream>

class SemanticAnalyzer {
private:
    std::unordered_map<std::string, std::string> symbolTable; // Stores (variable name -> type)
    std::unordered_map<std::string, bool> isConstant; // Stores (variable name -> const status)

public:
    void analyze(std::vector<std::shared_ptr<ASTNode>>& ast);

private:
    void analyzeNode(std::shared_ptr<ASTNode> node);
    void analyzeAssignment(std::shared_ptr<AssignmentNode> node);
    void analyzeVariableDeclaration(std::shared_ptr<VariableDeclarationNode> node);
    void analyzeFunctionDefinition(std::shared_ptr<FunctionDefinitionNode> node);
    void analyzeArrayAssignment(std::shared_ptr<ArrayAssignmentNode> node);
};

#endif // SEMANTIC_ANALYZER_H
