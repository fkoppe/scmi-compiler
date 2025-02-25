#include "analyzer.hpp"

#include <unordered_set>
#include <utility>

// void SemanticAnalyzer::analyze(std::vector<std::shared_ptr<ASTNode>>& ast) {
//     for (auto& node : ast) {
//         analyzeNode(node);
//     }
// }
pair<vector<FunctionDescr>,unordered_map<string,unordered_map<string,VariableType>>> analyze(vector<shared_ptr<ASTNode>>& nodes) {
    vector<FunctionDescr> function_descrs;
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

void checkFunctionNames(const vector<FunctionDescr>& function_descrs) {
    unordered_set<string> names;

    for (const FunctionDescr& x: function_descrs) {
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

SemanticAnalyzer::SemanticAnalyzer(const shared_ptr<FunctionDefinitionNode>& function_node, const vector<FunctionDescr>& function_descrs) {
    name = function_node->functionName;
    this->function_descrs = function_descrs;

    checkParams();

    for (const shared_ptr<ASTNode>& node : function_node->body) {
        if (const shared_ptr<VariableDeclarationNode> var = dynamic_pointer_cast<VariableDeclarationNode>(node)) {
            checkDeclaration(var);
            checkAssignment(make_shared<AssignmentNode>(make_shared<IdentifierNode>(var->varName), var->value));
        } else if (const shared_ptr<AssignmentNode> ass = dynamic_pointer_cast<AssignmentNode>(node)) {
            checkAssignment(ass);
        }
    }
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

VariableType SemanticAnalyzer::getVariableType(const shared_ptr<ASTNode>& node, const VariableType& expected_type) {
    if (shared_ptr<IdentifierNode> ident = dynamic_pointer_cast<IdentifierNode>(node)){
        checkIdentifier(ident);
        return findVariable(ident->name);
    }
    if (shared_ptr<FunctionCallNode> function_call = dynamic_pointer_cast<FunctionCallNode>(node)){
        FunctionDescr call_func = checkFunctionCall(function_call);
        return call_func.type;
    }
    if (shared_ptr<NumberNode> number_node = dynamic_pointer_cast<NumberNode>(node)) {
        if (expected_type.name == "int" && number_node->value < maxInt && number_node->value > minInt) {
            return {"int"};
        }
        if (expected_type.name == "short" && number_node->value < maxShort && number_node->value > minShort) {
            return {"short"};
        }
        if (expected_type.name == "char" && number_node->value < maxChar && number_node->value > minChar) {
            return {"char"};
        }
        cout << "Invalid number: " << number_node->value << " for type '" << expected_type.name << "' in function '" << this->name << "'" << endl;
        exit(-1);
    }
    throw runtime_error("Unrecognized node type for getVariableType");
}

FunctionDescr SemanticAnalyzer::findFunctionDescr(const string& name) {
    for (FunctionDescr x: function_descrs) {
        if (x.name == name) {
            return x;
        }
    }
    throw runtime_error("Function '" + name + "' not found");
}

VariableType SemanticAnalyzer::findVariable(const string & name) {
    auto it = variableList.find(name);
    if (it != variableList.end()) {
        return it->second;
    }
    throw runtime_error("Variable '" + name + "' not found");
}

FunctionDescr SemanticAnalyzer::checkFunctionCall(const shared_ptr<FunctionCallNode>& function_call_node) {
    FunctionDescr call_func;
    bool found = false;
    for (const FunctionDescr& x: function_descrs) {
        if (function_call_node->functionName == x.name) {
            found = true;
            call_func = x;
            break;
        }
    }
    if (!found) {
        cout << "Function call '" << function_call_node->functionName << "' in '" << this->name << "' not found" << endl;
        exit(-1);
    }

    //check params
    if (function_call_node->arguments.size() != call_func.params.size()) {
        cout << "Function call '" << function_call_node->functionName << "' in '" << this->name << "does not have the correct number of arguments" << endl;
    }

    for (int i = 0; i < function_call_node->arguments.size(); i++) {
        VariableType definition = call_func.params.at(i).second;
        VariableType input = getVariableType(function_call_node->arguments.at(i), definition);
        checkIdentifierType(function_call_node->functionName+"->"+call_func.params.at(i).first, definition, "<inputParameter>", input);
    }

    return call_func;
}

void SemanticAnalyzer::checkIdentifierType(string nameA, VariableType typeA, string nameB, VariableType typeB) {
    if (typeA.name != typeB.name) {
        cout << "Invalid type assignment with '" << nameA << "' [" << typeA.name << "] and '" << nameB << "' [" << typeB.name << "] in function '" << this->name << "'" << endl;
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

    VariableType definition = findVariable(ass->variable->name);
    VariableType input = getVariableType(ass->expression, definition);
    checkIdentifierType(ass->variable->name, definition, "<assignment>", input);
}


void SemanticAnalyzer::checkParams() {
    FunctionDescr ownDescr;

    for (const FunctionDescr& x: function_descrs) {
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
