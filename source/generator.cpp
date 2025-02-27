//
// Created by cedric on 21.02.25.
//
#include "generator.hpp"

#include <bits/locale_facets_nonio.h>

#include "ast.h"
#include "analyzer.hpp"

Function::Function(const shared_ptr<FunctionDefinitionNode>& functionNode, const unordered_map<string, Type>& variables, const vector<FunctionDescr>& function_descrs) {
    this->functionName = functionNode->functionName;
    this->function_descr_vector = function_descrs;
    this->function_descr_own = findFunctionDescr(functionName);
    this->returnLabel = functionName+"__return__";
    localVariablePointerOffset = 0;
    paramaterPointerOffset = 0;

    output += functionName + ":\n";

    if (functionName != "main") {
        output += "PUSHR\n";
    }

    output += "MOVE W SP,R13\n";

    //reset SP to sum of localVariableSize
    //assign addresses to localVariableMap (local + parameter)
    //...
    int declaredVariableSize = addVariables(variables);
    output += "SUB W I " + to_string(declaredVariableSize) + ",SP\n";



    for (const shared_ptr<ASTNode>& bodyElement: functionNode->body) {
        if (shared_ptr<VariableDeclarationNode> variable_declaration_node = dynamic_pointer_cast<VariableDeclarationNode>(bodyElement)) {
            generateAssignment(localVariableMap.at(variable_declaration_node->varName), variable_declaration_node->value);
        }
        else if (shared_ptr<AssignmentNode> assignment_node = dynamic_pointer_cast<AssignmentNode>(bodyElement)) {
            generateAssignment(localVariableMap.at(assignment_node->variable->name), assignment_node);
        }
        else if (shared_ptr<FunctionCallNode> function_call_node = dynamic_pointer_cast<FunctionCallNode>(bodyElement)) {
            if (function_call_node->functionName == OUTPUT_FUNCTION) {
                generateOutput(function_call_node);
                continue;
            }

            FunctionDescr function_descr = findFunctionDescr(function_call_node->functionName);
            generateFunctionCall(function_call_node, function_descr);

            //no need because function call as body-element is always void -> analyzer
            //output += "ADD W I " + to_string(function_descr.type.size()) + ",SP\n";
        }
        else if (const shared_ptr<ReturnValueNode>& return_value = dynamic_pointer_cast<ReturnValueNode>(bodyElement) ) {
            generateAssignment(localVariableMap.at("return"),return_value->value);
            output += "JUMP " + returnLabel+"\n";
        }
        else if (const shared_ptr<ReturnNode>& return_node = dynamic_pointer_cast<ReturnNode>(bodyElement)) {
            output += "JUMP " + returnLabel+"\n";
        }
    }

    output += returnLabel+":\n";
    output += "MOVE W R13,SP\n";
    if (functionName != "main") {
        output += "POPR\n";
    }
    output += "RET\n\n";
}

FunctionDescr Function::findFunctionDescr(const string& name) {
    for (auto & i : function_descr_vector) {
        if (i.name == name) {
            return i;
        }
    }
    cout << "cannot find function: " << name << endl;
    exit(-1);
}



string Function::getOutput() {
    return output;
}


void Function::generateFunctionCall(const shared_ptr<FunctionCallNode>& function_call_node, const FunctionDescr& function_call_type) {
    //reserve output space
    int outputSize = function_call_type.type.size();
    if (outputSize != 0) {
        output += "SUB W I " + to_string(function_call_type.type.size()) + ",SP\n";
    }

    int inputSize = 0;

    for (int i = function_call_type.params.size() - 1; i >= 0; i--) {
        Type paramType =  function_call_type.params.at(i).second;
        shared_ptr<ASTNode> arguments_node = function_call_node->arguments.at(i);

        generateAssignment({paramType, "-!SP"}, arguments_node);
        inputSize += paramType.size();
    }

    output += "CALL " + function_call_node->functionName + "\n";
    if (inputSize != 0) {
        output += "ADD W I " + to_string(inputSize) + ",SP\n";
    }
}

void Function::generateAssignment(const LocalVariable& assign_variable, const shared_ptr<ASTNode>& node_expression) {
    string assignment;
    Type assignType;

    if (const shared_ptr<NumberNode> numberNode = dynamic_pointer_cast<NumberNode>(node_expression)) {
        assignment = "I " + to_string(numberNode->value);
        assignType = assign_variable.type;
    }
    else if (const shared_ptr<IdentifierNode> identifier_node = dynamic_pointer_cast<IdentifierNode>(node_expression)) {
        LocalVariable local_variable = localVariableMap.at(identifier_node->name);
        assignment = local_variable.address;
        assignType = local_variable.type;
    }
    else if (const shared_ptr<FunctionCallNode> function_call_node = dynamic_pointer_cast<FunctionCallNode>(node_expression)) {
        FunctionDescr function_call_type = findFunctionDescr(function_call_node->functionName);
        generateFunctionCall(function_call_node, function_call_type);
        output += "MOVE " + function_call_type.type.miType() + " !SP+,R0\n";
        assignment = "R0";
        assignType = function_call_type.type;
    }
    else if (const shared_ptr<LogicalNode> logical_node = dynamic_pointer_cast<LogicalNode>(node_expression)) {
        vector<LogicalExpression> logical_expressions;
        getLogicalExpressions(logical_node, logical_expressions);

        for (const LogicalExpression& logical_expression : logical_expressions) {
            generateLogicalExpressions(logical_expression);
        }
        assignment = "!SP+";
        assignType = Type(TypeType::INT);
    }
    else {
        throw runtime_error("invalid assignment AST Node");
    }

    output += "MOVE " + assign_variable.type.miType() + " " + assignment + "," + assign_variable.address + "\n";

    if (assignType.getEnum() != assign_variable.type.getEnum()) {
        generateShift(assignType,assign_variable);
    }
}

int Function::addVariables(const unordered_map<string, Type>& variables) {
    unordered_set<string> params;
    params.reserve(function_descr_own.params.size());

    int paramOffset = 0;
    for (const auto&[name, type]:function_descr_own.params) {
        params.insert(name);
        string address = to_string(64+paramOffset)+"+!R13";
        paramOffset += type.size();
        localVariableMap[name] = {type, address};
    }

    //add return variable
    localVariableMap["return"] = {function_descr_own.type,to_string(64+paramOffset)+"+!R13"};


    int localOffset = 0;
    for (auto & [name, type]: variables) {
        if (params.count(name)) {
            continue;
        }
        localOffset += type.size();
        string address = "-"+to_string(localOffset)+"+!R13";
        localVariableMap[name] = {type, address};
    }
    return localOffset;
}

void Function::generateOutput(const shared_ptr<FunctionCallNode>& output) {
    for (int i = 0; i < output->arguments.size(); i++) {
        shared_ptr<IdentifierNode> identifier_node = dynamic_pointer_cast<IdentifierNode>(output->arguments.at(i));
        Type outputType = localVariableMap.at(identifier_node->name).type;
        generateAssignment({outputType,"R"+to_string(i)},output->arguments.at(i));
    }
}

LocalVariable Function::getLogicalExpressions(const shared_ptr<ASTNode>& node, vector<LogicalExpression>& output) {
    if (const shared_ptr<NumberNode> numberNode = dynamic_pointer_cast<NumberNode>(node)) {
        LocalVariable var = {Type(TypeType::INT), "I "+to_string(numberNode->value)};
        return var;
    }
    if (const shared_ptr<IdentifierNode> identifier_node = dynamic_pointer_cast<IdentifierNode>(node)) {
        LocalVariable local_variable = localVariableMap.at(identifier_node->name);
        return local_variable;
    }
    if (const shared_ptr<LogicalNode> log = dynamic_pointer_cast<LogicalNode>(node)) {
        output.push_back({getLogicalExpressions(log->left, output), getLogicalExpressions(log->right, output), log->logicalType});
        return {};
    }
    throw runtime_error("invalid logical expression AST Node");
}

void Function::generateLogicalExpressions(const LogicalExpression& logical_expression) {
    if (logical_expression.expression_L.address != "") {
        if (logical_expression.expression_L.type.getEnum() != TypeType::INT) {
            output += "MOVE W I 0,-!SP\n";
            output += "MOVE "+logical_expression.expression_L.type.miType()+" "+logical_expression.expression_L.address+",!SP\n";
            generateShift(logical_expression.expression_L.type, {Type(TypeType::INT), "!SP"});
        }
        else {
            output += "MOVE W "+logical_expression.expression_L.address+",-!SP\n";
        }
    }

    if (logical_expression.expression_R.address != "") {
        if (logical_expression.expression_R.type.getEnum() != TypeType::INT) {
            output += "MOVE W I 0,-!SP\n";
            output += "MOVE "+logical_expression.expression_R.type.miType()+" "+logical_expression.expression_R.address+",!SP\n";
            generateShift(logical_expression.expression_R.type, {Type(TypeType::INT), "!SP"});
        }
        else {
            output += "MOVE W "+logical_expression.expression_R.address+",-!SP\n";
        }
    }

    if (logical_expression.op == LogicalType::AND) {
        //ANDNOT s1,s2 => s2 && !s1
        output += "MOVEC W !SP,!SP\n";
        output += "ANDNOT W !SP,4+!SP\n";
        output += "ADD W I 4,SP\n";
    }
    else if (logical_expression.op == LogicalType::OR) {
        output += "OR W !SP,4+!SP\n";
        output += "ADD W I 4,SP\n";
    }
}

void Function::generateShift(const Type& from, const LocalVariable& to) {
    output += "SH I -"+to_string((to.type.size()-from.size())*8)+","+to.address+","+to.address+"\n";
}

string compile(const vector<shared_ptr<ASTNode>>& ast, const vector<FunctionDescr>& function_descrs, const unordered_map<string, unordered_map<string, Type>>& variables) {
    string output;
    output += "SEG\n";
    output += "MOVE W I H'00FFFF',SP\n";
    output += "CALL main\n";
    output += "HALT\n";
    for (int i = 0; i < ast.size(); i++) {
        shared_ptr<FunctionDefinitionNode> func = dynamic_pointer_cast<FunctionDefinitionNode>(ast[i]);
        Function function = Function(func, variables.at(func->functionName), function_descrs);
        output += function.getOutput();
    }
    output += "END";
    return output;
}
