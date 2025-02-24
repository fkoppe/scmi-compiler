//
// Created by cedric on 21.02.25.
//
#include "compiler.hpp"

#include <bits/locale_facets_nonio.h>

#include "ast.h"

Function::Function(const shared_ptr<FunctionDefinitionNode>& functionNode, const vector<string> &functionList) {
    this->functionList = functionList;
    this->functionName = functionNode->functionName;
    localVariablePointerOffset = 0;
    paramaterPointerOffset = 0;
    output += functionName + ":\n";
    output += "PUSHR\n";
    output += "MOVE W SP,R13\n";

    for (const pair<string,string>& parameter : functionNode->parameters) {
        addInputVariable(parameter);
    }

    for (const shared_ptr<ASTNode>& bodyElement: functionNode->body) {
        if (shared_ptr<VariableDeclarationNode> variable_declaration_node = dynamic_pointer_cast<VariableDeclarationNode>(bodyElement)) {
            addLocalVariable(*variable_declaration_node);
        }
    }

    output += "MOVE W R13,SP\n";
    output += "POPR\n";
    output += "RET\n";
}

void Function::addInputVariable(const pair<string,string>& parameter) {
    checkForbiddenIdentifier(parameter.second);

    const string address = to_string(paramaterPointerOffset + 64)+"+!R13";
    paramaterPointerOffset += getSize(parameter.first);

    variableList.push_back({parameter.second,address});
}

void Function::addLocalVariable(const VariableDeclarationNode& declaration_node) {
    checkForbiddenIdentifier(declaration_node.varName);

    string declarationValue;

    if (const shared_ptr<NumberNode> numberNode = dynamic_pointer_cast<NumberNode>(declaration_node.value)) {
        declarationValue = "I " + to_string(numberNode->value);
    } else if (const shared_ptr<IdentifierNode> identifier_node = dynamic_pointer_cast<IdentifierNode>(declaration_node.value)) {
        const Variable var = findVariable(identifier_node->name);
        declarationValue = var.address;
    }
    else {
        return;
    }

    const int varSize = getSize(declaration_node.varType);
    localVariablePointerOffset += varSize;
    const string address = "-"+to_string(localVariablePointerOffset) + "+!R13";
    variableList.push_back({declaration_node.varName, address});


    output += "SUB W I " + to_string(varSize) + ",SP\n";
    output += "MOVE " + getMiType(declaration_node.varType) + " " + declarationValue + "," + address + "\n";
}

Variable Function::findVariable(const string& name) {
    for (auto & i : variableList) {
        if (i.name == name) {
            return i;
        }
    }
    cout << "cannot find identifier: " << name << endl;
    exit(-1);
}

void checkForbiddenIdentifier(const string& name) {
    if (FORBIDDEN_IDENTIFIER_NAMES.count(name)) {
        cout << "forbidden identifier name:" << name << endl;
        exit(-1);
    }
}

string Function::getOutput() {
    return output;
}

string Function::getFunctionName() {
    return functionName;
}

int getSize(const string& type) {
    if (type == "int") {
        return 4;
    }
    if (type == "short") {
        return 2;
    }
    if (type == "char") {
        return 1;
    }
    if (type == "float") {
        return 4;
    }
    if (type == "double") {
        return 8;
    };

    cout << "invalid type: " << type << "for getSize" << endl;
    exit(-1);
}

string getMiType(const string& type) {
    if (type == "int") {
        return "W";
    }
    if (type == "short") {
        return "H";
    }
    if (type == "char") {
        return "B";
    }
    if (type == "float") {
        return "F";
    }
    if (type == "double") {
        return "D";
    };

    cout << "invalid type: " << type << "for getSize" << endl;
    exit(-1);
}

string compile(const vector<shared_ptr<ASTNode>>& ast) {
    vector<string> functionNames;

    string output;

    string startOutput;
    startOutput += "SEG\n";
    startOutput += "MOVE W I H'00FFFF',SP\n";
    startOutput += "JUMP main\n";


    vector<shared_ptr<FunctionDefinitionNode>> functionDefinitions;
    functionDefinitions.reserve(ast.size());

    for (const shared_ptr<ASTNode>& i: ast) {
        if (const shared_ptr<FunctionDefinitionNode> function = dynamic_pointer_cast<FunctionDefinitionNode>(i)) {
            functionDefinitions.push_back(function);
        }
        else {
            cout << "root AST nodes are not function declarations";
        }
    }

    if (functionDefinitions.empty()) {
        cout << "Error: No function definitions found!" << endl;
        exit(-1);
    }

    for (int i = 0; i < functionDefinitions.size()-1; i++) {
        Function function = Function(functionDefinitions[i], functionNames);
        output += function.getOutput();
        functionNames.push_back(function.getFunctionName());
    }

    Function main = Function(functionDefinitions[functionDefinitions.size() - 1], functionNames);
    if (main.getOutput().substr(0,5) != "main:") {
        cout << "main() not detected";
    }

    output = startOutput + main.getOutput() + output;

    output += "END";
    return output;
}
