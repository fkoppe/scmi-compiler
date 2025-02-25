#include "semantic_analyzer.hpp"

void SemanticAnalyzer::analyze(vector<shared_ptr<ASTNode>>& ast) {
    for (auto& node : ast) {
        analyzeNode(node);
    }
}

// Analyze a single AST node
void SemanticAnalyzer::analyzeNode(shared_ptr<ASTNode> node) {
    if (auto assignment = dynamic_pointer_cast<AssignmentNode>(node)) {
        analyzeAssignment(assignment);
    } else if (auto varDecl = dynamic_pointer_cast<VariableDeclarationNode>(node)) {
        analyzeVariableDeclaration(varDecl);
    } else if (auto funcDef = dynamic_pointer_cast<FunctionDefinitionNode>(node)) {
        analyzeFunctionDefinition(funcDef);
    } else if (auto arrayAssign = dynamic_pointer_cast<ArrayAssignmentNode>(node)) {
        analyzeArrayAssignment(arrayAssign);
    }
}

// Check assignments for constant modifications or undeclared variables
void SemanticAnalyzer::analyzeAssignment(shared_ptr<AssignmentNode> node) {
    string varName = node->variable->name;

    if (symbolTable.find(varName) == symbolTable.end()) {
        cerr << "Semantic Error: Variable '" << varName << "' is used before declaration.\n";
        exit(1);
    }

    if (isConstant[varName]) {
        cerr << "Semantic Error: Cannot assign a new value to constant variable '" << varName << "'.\n";
        exit(1);
    }
}

// Check for duplicate variable declarations
void SemanticAnalyzer::analyzeVariableDeclaration(shared_ptr<VariableDeclarationNode> node) {
    string varName = node->varName;
    string varType = node->varType;

    if (symbolTable.find(varName) != symbolTable.end()) {
        cerr << "Semantic Error: Variable '" << varName << "' is already declared.\n";
        exit(1);
    }

    symbolTable[varName] = varType;
    isConstant[varName] = (varType == "const"); // Track if the variable is a constant
}

// Check function definitions for duplicate parameters or duplicate function names
void SemanticAnalyzer::analyzeFunctionDefinition(shared_ptr<FunctionDefinitionNode> node) {
    if (symbolTable.find(node->functionName) != symbolTable.end()) {
        cerr << "Semantic Error: Function '" << node->functionName << "' is already declared.\n";
        exit(1);
    }

    symbolTable[node->functionName] = "function"; // Register function name

    unordered_map<string, bool> localScope;
    for (auto& param : node->parameters) {
        if (localScope.find(param.second) != localScope.end()) {
            cerr << "Semantic Error: Duplicate parameter name '" << param.second << "' in function '" << node->functionName << "'.\n";
            exit(1);
        }
        localScope[param.second] = true;
    }

    for (auto& stmt : node->body) {
        analyzeNode(stmt); // Recursively analyze function body
    }
}

// Check for out-of-bounds errors or assigning to undeclared arrays
void SemanticAnalyzer::analyzeArrayAssignment(shared_ptr<ArrayAssignmentNode> node) {
    string arrayName = node->arrayName->name;

    if (symbolTable.find(arrayName) == symbolTable.end()) {
        cerr << "Semantic Error: Array '" << arrayName << "' is used before declaration.\n";
        exit(1);
    }

    if (isConstant[arrayName]) {
        cerr << "Semantic Error: Cannot modify constant array '" << arrayName << "'.\n";
        exit(1);
    }
}
