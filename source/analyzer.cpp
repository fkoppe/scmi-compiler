#include "analyzer.hpp"

#include <unordered_set>
#include <utility>

pair<vector<FunctionDescr>,unordered_map<string,unordered_map<string,Type>>> analyze(vector<shared_ptr<ASTNode>>& nodes) {
    vector<FunctionDescr> function_descrs;
    unordered_map<string, unordered_map<string,Type>> mapVariableList;

    vector<shared_ptr<FunctionDefinitionNode>> functions;

    for (const shared_ptr<ASTNode>& ast : nodes) {
        if (const shared_ptr<FunctionDefinitionNode> func = dynamic_pointer_cast<FunctionDefinitionNode>(ast)) {
            functions.push_back(func);

            vector<pair<string,Type>> paramVariables;
            for (const pair<Type,string>& p: func->parameters) {
                paramVariables.push_back({p.second, {p.first}});
            }
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
        }
        else if (const shared_ptr<AssignmentNode> ass = dynamic_pointer_cast<AssignmentNode>(node)) {
            checkAssignment(ass);
        }
        else if (const shared_ptr<FunctionCallNode> function_call = dynamic_pointer_cast<FunctionCallNode>(node)) {
            FunctionDescr function_descr = checkFunctionCall(function_call);
            checkIdentifierType(function_call->functionName, function_descr.type, "", Type(TypeType::VOID));
        }
        else if (const shared_ptr<ReturnValueNode>& return_value = dynamic_pointer_cast<ReturnValueNode>(node) ) {
            Type definition = function_node->returnType;
            Type input = getVariableType(return_value->value, definition);
            checkIdentifierType(function_node->functionName, definition, "<returnValue>", input);
        }
        else if (const shared_ptr<ReturnNode>& return_node = dynamic_pointer_cast<ReturnNode>(node)) {
            if (function_node->returnType.getEnum() != TypeType::VOID) {
                cout << "return value ["<< function_node->returnType.toString() <<"] need to be specified in '"<< function_node->functionName <<"'" << endl;
                exit(-1);
            }
        }
        else if (const shared_ptr<IfNode> if_node = dynamic_pointer_cast<IfNode>(node)) {
            //checkCondition
            checkCondition(if_node->condition);
        }
    }
}

unordered_map<string, Type> SemanticAnalyzer::getVariableList() {
    return variableList;
}

string SemanticAnalyzer::getName() {
    return name;
}


void SemanticAnalyzer::checkDeclaration(const shared_ptr<VariableDeclarationNode> &var) {
    checkForbiddenIdentifier(var->varName);

    Type type = {var->varType};
    if (!variableList.try_emplace(var->varName, type).second) {
        cout << "Variable '" << var->varName << "' in function '" << name << "' is already declared" << endl;
        exit(-1);
    }
}

Type SemanticAnalyzer::getCastType(Type found, Type expected) {
    int foundNum = static_cast<int>(found.getEnum());
    int expectedNum = static_cast<int>(expected.getEnum());

    if (foundNum <= expectedNum) {
        return expected;
    }
    return found;
}

Type SemanticAnalyzer::getVariableType(const shared_ptr<ASTNode>& node, const Type& expected_type) {
    if (shared_ptr<IdentifierNode> ident = dynamic_pointer_cast<IdentifierNode>(node)){
        checkIdentifier(ident);
        Type foundType = findVariable(ident->name);
        return getCastType(foundType, expected_type);
    }
    if (shared_ptr<FunctionCallNode> function_call = dynamic_pointer_cast<FunctionCallNode>(node)){
        FunctionDescr call_func = checkFunctionCall(function_call);
        return getCastType(call_func.type, expected_type);
    }
    if (shared_ptr<NumberNode> number_node = dynamic_pointer_cast<NumberNode>(node)) {
        if (expected_type.getEnum() == TypeType::INT && number_node->value < maxInt && number_node->value > minInt) {
            return Type(TypeType::INT);
        }
        if (expected_type.getEnum() == TypeType::SHORT && number_node->value < maxShort && number_node->value > minShort) {
            return Type(TypeType::SHORT);
        }
        if (expected_type.getEnum() == TypeType::CHAR && number_node->value < maxChar && number_node->value > minChar) {
            return Type(TypeType::CHAR);
        }
        cout << "Invalid number: " << number_node->value << " for type '" << expected_type.toString() << "' in function '" << this->name << "'" << endl;
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

Type SemanticAnalyzer::findVariable(const string & name) {
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

    if (function_call_node->functionName == OUTPUT_FUNCTION && function_call_node->arguments.size() <= 13) {
        found = true;
        vector<pair<string,Type>> params;

        for (const auto& x: function_call_node->arguments) {
            if (shared_ptr<IdentifierNode> identifier_node = dynamic_pointer_cast<IdentifierNode>(x)) {
                params.emplace_back("",variableList.at(identifier_node->name));
            }
            else {
                found = false;
            }
        }

        call_func = {function_call_node->functionName, Type(TypeType::VOID), params};
    }


    if (!found) {
        cout << "Function call '" << function_call_node->functionName << "' in '" << this->name << "' not found" << endl;
        exit(-1);
    }

    //check params
    if (function_call_node->arguments.size() != call_func.params.size()) {
        cout << "Function call '" << function_call_node->functionName << "' in '" << this->name << "' does not have the correct number of arguments" << endl;
    }

    for (int i = 0; i < function_call_node->arguments.size(); i++) {
        Type definition = call_func.params.at(i).second;
        Type input = getVariableType(function_call_node->arguments.at(i), definition);
        checkIdentifierType(function_call_node->functionName+"->"+call_func.params.at(i).first, definition, "<inputParameter>", input);
    }

    return call_func;
}

void SemanticAnalyzer::checkIdentifierType(string nameA, Type typeA, string nameB, Type typeB) {
    if (typeA.getEnum() != typeB.getEnum()) {
        cout << "Invalid type assignment with '" << nameA << "' [" << typeA.toString() << "] and '" << nameB << "' [" << typeB.toString() << "] in function '" << this->name << "'" << endl;
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

    Type definition = findVariable(ass->variable->name);
    Type input = getVariableType(ass->expression, definition);
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

void SemanticAnalyzer::checkCondition(const shared_ptr<ASTNode>& condition_node) {
}

void checkForbiddenIdentifier(const string& name) {
    if (FORBIDDEN_IDENTIFIER_NAMES.count(name)) {
        cout << "forbidden identifier name:" << name << endl;
        exit(-1);
    }

    for (const auto& x : FORBIDDEN_SUBSTRING) {
        if (name.find(x) != std::string::npos) {
            std::cout << "forbidden substring found in identifier: " << name << std::endl;
            exit(-1);
        }
    }
}
