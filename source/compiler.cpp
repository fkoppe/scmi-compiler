//
// Created by cedric on 21.02.25.
//
#include "compiler.hpp"

#include <bits/locale_facets_nonio.h>

#include "ast.h"
#include "semantic_analyzer.hpp"

Function::Function(const shared_ptr<FunctionDefinitionNode>& functionNode, const unordered_map<string, VariableType>& variables, const vector<FunctionDescr>& function_descrs) {
    this->function_descrs = function_descrs;
    localVariablePointerOffset = 0;
    paramaterPointerOffset = 0;

    //reset SP to sum of localVariableSize
    //assign addresses to localVariableMap (local + parameter)
    //...


    output += functionNode->functionName + ":\n";
    output += "PUSHR\n";
    output += "MOVE W SP,R13\n";


    for (const shared_ptr<ASTNode>& bodyElement: functionNode->body) {
        if (shared_ptr<VariableDeclarationNode> variable_declaration_node = dynamic_pointer_cast<VariableDeclarationNode>(bodyElement)) {
            //addLocalVariable(*variable_declaration_node);
        }
        if (shared_ptr<FunctionCallNode> function_call_node = dynamic_pointer_cast<FunctionCallNode>(bodyElement)) {
            FunctionDescr function_descr = findFunctionDescr(function_call_node->functionName);
            output += getFunctionCall(function_call_node, function_descr);
            //output += "ADD W I " + to_string(function_descr.type.size) + ",SP\n";
        }
        if (shared_ptr<AssignmentNode> assignment_node = dynamic_pointer_cast<AssignmentNode>(bodyElement)) {
            //output += getAssigment(findVariable(assignment_node->variable->name), assignment_node->expression);
        }
    }

    output += "MOVE W R13,SP\n";
    output += "POPR\n";
    output += "RET\n";
}

FunctionDescr Function::findFunctionDescr(const string& name) {
    for (auto & i : function_descrs) {
        if (i.name == name) {
            return i;
        }
    }
    cout << "cannot find function: " << name << endl;
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


string Function::getFunctionCall(const shared_ptr<FunctionCallNode>& function_call_node, const FunctionDescr& function_call_type) {
    string result;
    //const int outputSize = function_call_type.type.size;

    vector<LocalVariable> inputs;

    for (const shared_ptr<ASTNode>& arguments_node: function_call_node->arguments) {
        if (shared_ptr<NumberNode> arguments_number = dynamic_pointer_cast<NumberNode>(arguments_node)) {
            //inputs.push_back({"", "I " + to_string(arguments_number->value), {"int", 4}});
        }
        if (shared_ptr<IdentifierNode> arguments_Identifier = dynamic_pointer_cast<IdentifierNode>(arguments_node)) {
            //inputs.push_back(findVariable(arguments_Identifier->name));
        }
    }

    // if (outputSize != 0) {
    //     result += "SUB W I " + to_string(outputSize) + ",SP\n";
    // }

    // int inputSize = 0;
    // if (inputs.size() > 0) {
    //     for (int i = inputs.size() - 1; i >= 0; i--) {
    //         const Variable& variable = inputs[i];
    //         inputSize += variable.type.size;
    //         result += "MOVE " + getMiType(variable.type.name) + " " + variable.address + ",-!SP\n";
    //     }
    // }

    result += "CALL " + function_call_node->functionName + "\n";
    //result += "ADD W I " + to_string(inputSize) + ",SP\n";
    return result;
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
    if (type == "void") {
        return 0;
    }

    cout << "invalid type: " << type << " for getSize" << endl;
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

    cout << "invalid type: " << type << " for getSize" << endl;
    exit(-1);
}

string Function::getAssigment(const LocalVariable& assign_variable, const shared_ptr<ASTNode>& node_expression) {
    string result;
    string declarationValue;

    if (const shared_ptr<NumberNode> numberNode = dynamic_pointer_cast<NumberNode>(node_expression)) {
        declarationValue = "I " + to_string(numberNode->value);
    } else if (const shared_ptr<IdentifierNode> identifier_node = dynamic_pointer_cast<IdentifierNode>(node_expression)) {
        //const Variable var = findVariable(identifier_node->name);
        //declarationValue = var.address;
    } else if (const shared_ptr<FunctionCallNode> function_call_node = dynamic_pointer_cast<FunctionCallNode>(node_expression)) {
        FunctionDescr function_call_type = findFunctionDescr(function_call_node->functionName);
        output += getFunctionCall(function_call_node, function_call_type);
        output += "MOVE " + getMiType(function_call_type.type.name) + " !SP+,R0\n";
        declarationValue = "R0";
    }
    else {
        return "";
    }

    result += "MOVE " + getMiType(assign_variable.type.name) + " " + declarationValue + "," + assign_variable.address + "\n";
    return result;
}

string compile(const vector<shared_ptr<ASTNode>>& ast, const vector<FunctionDescr>& function_descrs, const unordered_map<string, unordered_map<string, VariableType>>& variables) {
    string output;
    output += "SEG\n";
    output += "MOVE W I H'00FFFF',SP\n";
    output += "CALL main\n";
    output += "HALT\n";
    for (int i = 0; i < ast.size()-1; i++) {
        shared_ptr<FunctionDefinitionNode> func = dynamic_pointer_cast<FunctionDefinitionNode>(ast[i]);
        Function function = Function(func, variables.at(func->functionName), function_descrs);
        output += function.getOutput();
    }
    output += "END";
    return output;
}
