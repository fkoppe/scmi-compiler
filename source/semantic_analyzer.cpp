#include "semantic_analyzer.hpp"

#include <unordered_set>
#include <utility>

// void SemanticAnalyzer::analyze(std::vector<std::shared_ptr<ASTNode>>& ast) {
//     for (auto& node : ast) {
//         analyzeNode(node);
//     }
// }
pair<vector<S_FunctionDescr>,unordered_map<string,unordered_map<string,VariableType>>> analyze(vector<shared_ptr<ASTNode>>& nodes) {
    vector<S_FunctionDescr> function_descrs;
    unordered_map<string, unordered_map<string,VariableType>> mapVariableList;

    vector<shared_ptr<FunctionDefinitionNode>> functions;

    for (const shared_ptr<ASTNode>& ast : nodes) {
        if (const shared_ptr<FunctionDefinitionNode> func = dynamic_pointer_cast<FunctionDefinitionNode>(ast)) {
            functions.push_back(func);

            vector<pair<string,VariableType>> paramVariables;
            for (const pair<string,string>& p: func->parameters) {
                paramVariables.push_back({p.second, {p.first}});
            }
            //TODO: type
            function_descrs.push_back({func->functionName, {func->returnType}, paramVariables});
        }
        else {
            cout << "Function declaration in AST Node not found" << endl;
        }
    }

    //loop through function_descrs and check for same name and if main exist
    checkFunctionNames(function_descrs);

    for (shared_ptr<FunctionDefinitionNode> &node : functions) {
        SemanticAnalyzer analyzer = SemanticAnalyzer(node, function_descrs);
        mapVariableList.insert_or_assign(analyzer.getName(),analyzer.getVariableList());
    }

    //                      functionDescr         name      variableList
    return {function_descrs, mapVariableList};
}

void checkFunctionNames(const vector<S_FunctionDescr>& function_descrs) {
    unordered_set<string> names;

    for (const S_FunctionDescr& x: function_descrs) {
        if (names.count(x.name)) {
            cout << "Function name: '" << x.name << "' already exists" << endl;
            exit(-1);
        }
        names.insert(x.name);
    }

    if (!names.count("main")) {
        cout << "Cannot find 'main' function" << endl;
        exit(-1);
    }
}

SemanticAnalyzer::SemanticAnalyzer(const shared_ptr<FunctionDefinitionNode>& function_node, const vector<S_FunctionDescr>& function_descrs) {
    name = function_node->functionName;
    this->function_descrs = function_descrs;

    checkParams();

    for (const shared_ptr<ASTNode>& node : function_node->body) {
        if (const shared_ptr<VariableDeclarationNode> var = dynamic_pointer_cast<VariableDeclarationNode>(node)) {
            checkDeclaration(var);
        } else if (const shared_ptr<AssignmentNode> ass = dynamic_pointer_cast<AssignmentNode>(node)) {
            checkAssignment(ass);
        }
    }
    name = name;

}

unordered_map<string, VariableType> SemanticAnalyzer::getVariableList() {
    return variableList;
}

string SemanticAnalyzer::getName() {
    return name;
}


void SemanticAnalyzer::checkDeclaration(const shared_ptr<VariableDeclarationNode> &var) {
    checkForbiddenIdentifier(var->varName);

    VariableType type = {var->varType};
    //TODO: type
    if (!variableList.try_emplace(var->varName, type).second) {
        cout << "Variable '" << var->varName << "' in function '" << name << "' is already declared" << endl;
        exit(-1);
    }
}

VariableType SemanticAnalyzer::findVariable(const string & name) {
    auto it = variableList.find(name);
    if (it != variableList.end()) {
        return it->second;
    }
    throw runtime_error("Variable '" + name + "' not found");
}

void SemanticAnalyzer::checkIdentifierType(IdentifierNode a, IdentifierNode b) {
    VariableType typeA = findVariable(a.name);
    VariableType typeB = findVariable(b.name);
    if (typeA.name != typeB.name) {
        cout << "Invalid type assignment with '" << a.name << "' [" << typeA.name << "] and '" << b.name << "' [" << typeB.name << "] in function '" << this->name << "'" << endl;
        exit(-1);
    }
}

void SemanticAnalyzer::checkIdentifier(const shared_ptr<IdentifierNode>& identifier) {
    if (!variableList.count(identifier->name)) {
        cout << "Variable '" << identifier->name << "' in function '" << name << "' does not exist" << endl;
        exit(-1);
    }
}

void SemanticAnalyzer::checkAssignment(const shared_ptr<AssignmentNode>& ass) {
    checkIdentifier(ass->variable);

    if (shared_ptr<IdentifierNode> ident = dynamic_pointer_cast<IdentifierNode>(ass->expression)){
        checkIdentifier(ident);
        checkIdentifierType(*ass->variable, *ident);
    }

}


void SemanticAnalyzer::checkParams() {
    S_FunctionDescr ownDescr;

    for (const S_FunctionDescr& x: function_descrs) {
        if (x.name == name) {
            ownDescr = x;
        }
    }

    for (const auto&[name, type]: ownDescr.params) {
        checkForbiddenIdentifier(name);
        if (!variableList.try_emplace(name, type).second) {
            cout << "Parameter '" << name << "' for function '"<< this->name << "' already exists" << endl;
        }
    }
}
