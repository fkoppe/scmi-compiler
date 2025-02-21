//
// Created by cedric on 21.02.25.
//
#include "compiler.hpp"

#include <bits/locale_facets_nonio.h>

#include "ast.h"

Function::Function(const FunctionDefinitionNode& functionNode) {
    localVariablePointerOffset = 0;
    // Constructor implementation (initialize function-related things)
}



void Function::addLocalVariable(const VariableDeclarationNode& declaration_node) {
    /*if (declaration_node.value typeof NumberNode) {
        NumberNode node = (NumberNode) declaration_node.value;
    }*/

    if (const shared_ptr<NumberNode> numberNode = dynamic_pointer_cast<NumberNode>(declaration_node.value)) {
        const int varSize = getSize(declaration_node.varType);
        localVariablePointerOffset += varSize;
        string address = "-"+to_string(localVariablePointerOffset) + "+!R13";
        variableList.push_back({declaration_node.varName, address});

        output += "SUB W I " + to_string(varSize) + ",SP\n";
        output += "MOVE " + getMiType(declaration_node.varType) + " I " + to_string(numberNode->value) + "," + address + "\n";
    }

    if (const shared_ptr<IdentifierNode> identifier_node = dynamic_pointer_cast<IdentifierNode>(declaration_node.value)) {
        const int varSize = getSize(declaration_node.varType);
        localVariablePointerOffset += varSize;
        string address = "-"+to_string(localVariablePointerOffset) + "+!R13";
        variableList.push_back({declaration_node.varName, address});

        const Variable var = findVariable(identifier_node->name);

        output += "SUB W I " + to_string(varSize) + ",SP\n";
        output += "MOVE " + getMiType(declaration_node.varType) + var.address + "," + address + "\n";
    }
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
    // Implement your compile logic here
    return "";
}
