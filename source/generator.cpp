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
    this->localVariablePointerOffset = 0;
    this->paramaterPointerOffset = 0;
    this->jumpLabelNum = 0;
    this->registerNum = 0;


    output += functionName + ":\n";

    if (functionName != "main") {
        output += "PUSHR\n";
    }

    output += "MOVE W SP,R13\n";

    int declaredVariableSize = addVariables(variables);
    output += "SUB W I " + to_string(declaredVariableSize) + ",SP\n";

    generateNodes(functionNode->body);

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
        string outputRegister = getNextRegister();

        output += "MOVE " + function_call_type.type.miType() + " !SP+,"+outputRegister+"\n";
        assignment = outputRegister;
        assignType = function_call_type.type;
    }
    else if (const shared_ptr<LogicalNode> logical_node = dynamic_pointer_cast<LogicalNode>(node_expression)) {
        vector<MathExpression> logical_expressions;
        getMathExpression(logical_node, logical_expressions);

        for (const MathExpression& arithmetic_expression : logical_expressions) {
            if (holds_alternative<LogicalType>(arithmetic_expression.op)) {
                generateLogicalExpression(arithmetic_expression);
            }
            if (holds_alternative<ArithmeticType>(arithmetic_expression.op)) {
                generateArithmeticExpression(arithmetic_expression, assign_variable.type);
            }
        }
        assignment = "!SP+";
        assignType = Type(TypeType::INT);
    }
    else if (const shared_ptr<ArithmeticNode>& arithmetic_node = dynamic_pointer_cast<ArithmeticNode>(node_expression)) {
        vector<MathExpression> arithmetic_expressions;
        getMathExpression(arithmetic_node, arithmetic_expressions);

        for (const MathExpression& arithmetic_expression : arithmetic_expressions) {
            if (holds_alternative<LogicalType>(arithmetic_expression.op)) {
                generateLogicalExpression(arithmetic_expression);
            }
            if (holds_alternative<ArithmeticType>(arithmetic_expression.op)) {
                generateArithmeticExpression(arithmetic_expression, assign_variable.type);
            }
        }
        assignment = "!SP+";
        assignType = assign_variable.type;
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

void Function::generateLogicalExpression(const MathExpression& logical_expression) {
    LogicalType logType = get<LogicalType>(logical_expression.op);

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

    if (logType == LogicalType::AND) {
        //ANDNOT s1,s2 => s2 && !s1
        output += "MOVEC W !SP,!SP\n";
        output += "ANDNOT W !SP,4+!SP\n";
        output += "ADD W I 4,SP\n";
    }
    else if (logType == LogicalType::OR) {
        output += "OR W !SP,4+!SP\n";
        output += "ADD W I 4,SP\n";
    }
    else {
        output += "CMP W 4+!SP,!SP\n";
        string trueLabel = getNextJumpLabel();
        string falseLabel = getNextJumpLabel();
        output += getCompareJump(logType) + " " + trueLabel+"\n";
        //false
        output += "MOVE W I 0,4+!SP\n";
        output += "JUMP "+falseLabel+"\n";

        //true
        output += trueLabel+":\n";
        output += "MOVE W I 1,4+!SP\n";

        output += falseLabel+":\n";
        output += "ADD W I 4,SP\n";
    }
}

void Function::generateShift(const Type& from, const LocalVariable& to) {
    output += "SH I -"+to_string((to.type.size()-from.size())*8)+","+to.address+","+to.address+"\n";
}

string Function::getCompareJump(const LogicalType& logical) {
    switch (logical) {
        case LogicalType::EQUAL:
            return "JEQ";
        case LogicalType::NOT_EQUAL:
            return "JNE";
        case LogicalType::LESS_THAN:
            return "JLT";
        case LogicalType::GREATER_THAN:
            return "JGT";
        case LogicalType::LESS_EQUAL:
            return "JLE";
        case LogicalType::GREATER_EQUAL:
            return "JGE";
        default:
            return "";
    }
}

string Function::getNextJumpLabel() {
    string output = this->functionName+"__jump__"+to_string(jumpLabelNum);
    jumpLabelNum++;
    return output;
}

void Function::generateNodes(const vector<shared_ptr<ASTNode>>& node) {
    for (const shared_ptr<ASTNode>& bodyElement: node) {
        if (shared_ptr<VariableDeclarationNode> variable_declaration_node = dynamic_pointer_cast<VariableDeclarationNode>(bodyElement)) {
            generateAssignment(localVariableMap.at(variable_declaration_node->varName), variable_declaration_node->value);
        }
        else if (shared_ptr<AssignmentNode> assignment_node = dynamic_pointer_cast<AssignmentNode>(bodyElement)) {
            generateAssignment(localVariableMap.at(assignment_node->variable->name), assignment_node->expression);
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
        else if (const shared_ptr<IfNode>& if_node = dynamic_pointer_cast<IfNode>(bodyElement)) {
            string trueLabel = getNextJumpLabel();
            string continueLabel = getNextJumpLabel();

            generateAssignment({Type(TypeType::INT), "-!SP"},if_node->condition);

            output += "MOVE W I 0,-!SP\n";
            output += "CMP W !SP,4+!SP\n";
            output += "JNE "+trueLabel+"\n";
            generateNodes(if_node->elseBlock);
            output += "JUMP "+continueLabel+"\n";
            output += trueLabel+":\n";
            generateNodes(if_node->thenBlock);
            output += continueLabel+":\n";
        }
        else if (const shared_ptr<ArrayDeclarationNode> arr = dynamic_pointer_cast<ArrayDeclarationNode>(bodyElement)) {
            LocalVariable local_variable = localVariableMap.at(arr->name);
            Type arrayElementType = convertArrayToVarType(arr->type);
            int elementSize = 0;
            if (arr->size == -1) {
                elementSize = arr->arrayValues.size();
            }
            else {
                elementSize = arr->size;
            }
            int arraySize = elementSize * arrayElementType.size();
            malloc(arraySize, local_variable.address);

            if (arr->size == -1) {
                //fill values
                for (int i = 0; i < arr->arrayValues.size(); i++) {
                    generateArrayIndexAssignment(local_variable, i);
                    generateAssignment({arrayElementType,"!R0"}, arr->arrayValues.at(i));
                    clearRegisterNum();
                }
            }
        }
    }
}

void Function::generateArithmeticExpression(const MathExpression& arithmetic_expression, const Type& expected_type) {
    ArithmeticType ariType = get<ArithmeticType>(arithmetic_expression.op);

    LocalVariable l = arithmetic_expression.expression_L;
    LocalVariable r = arithmetic_expression.expression_R;
    if (l.address != "") {
        if (l.type.miType() != expected_type.miType()) {
            output += "MOVE "+expected_type.miType()+" I 0,-!SP\n";
            output += "MOVE "+ l.type.miType() + " " + l.address + ",!SP\n";
            generateShift(l.type, {expected_type,"!SP"});
        }
        else {
            output += "MOVE "+ expected_type.miType() + " " + l.address + ",-!SP\n";
        }
    }

    if (arithmetic_expression.expression_R.address != "") {
        if (r.type.miType() != expected_type.miType()) {
            output += "MOVE "+expected_type.miType()+" I 0,-!SP\n";
            output += "MOVE "+ r.type.miType() + " " + r.address + ",!SP\n";
            generateShift(r.type, {expected_type,"!SP"});
        }
        else {
            output += "MOVE "+ expected_type.miType() + " " + r.address + ",-!SP\n";
        }
    }
    generateArithmeticOperation(ariType, expected_type);

}

void Function::generateArithmeticOperation(const ArithmeticType arithmetic, const Type type) {
    if (arithmetic == ArithmeticType::MODULO) {
        //b => !SP
        //a => 4+!SP
        //
        //a mod b =>
        //temp = a div b
        //temp = b mult temp
        //erg = a - temp
        output += "DIV "+type.miType()+" !SP,4+!SP,R0\n"; //temp = a div b
        output += "MULT "+type.miType()+" !SP,R0\n"; // temp = b mult temp
        output += "SUB "+type.miType()+" R0,4+!SP\n"; // erg = a -temp
        output += "ADD W I 4,SP\n";
        return;
    }



    string op = "";

    switch (arithmetic) {
        case ArithmeticType::ADD:
            op = "ADD";
            break;
        case ArithmeticType::SUBTRACT:
            op = "SUB";
            break;
        case ArithmeticType::MULTIPLY:
            op = "MULT";
            break;
        case ArithmeticType::DIVIDE:
            op = "DIV";
            break;
        default: op = "";
    }
    /*
    a - b

    stack:
    b
    a
    */
    output += op +" "+type.miType()+" !SP,4+!SP\n";
    output += "ADD W I 4,SP\n";
}

static void generateMallocFunction(string& output) {
    output += "__malloc__:\n";
    output += "MOVE W HP,8+!SP\n";
    output += "ADD W 4+!SP,HP\n";
    output += "RET\n";
}

void Function::malloc(int size, const string& assignment) {
    output += "MOVE W I 0,-!SP\n";
    output += "MOVE W I "+ to_string(size) + ",-!SP\n";
    output += "CALL __malloc__\n";
    output += "ADD W I 4, SP\n";
    output += "MOVE W !SP+,"+assignment+"\n";
}

void Function::generateArrayIndexAssignment(const LocalVariable& array, const int index) {
    string outputRegister = getNextRegister();
    output += "MOVE W "+array.address + ","+outputRegister+"\n";
    output += "ADD W I "+to_string(index*convertArrayToVarType(array.type).size())+","+outputRegister+"\n";
}

string Function::getNextRegister() {
    string output = "R"+to_string(registerNum);
    registerNum++;
    if (registerNum > 12) {
        cout << "register overflow" << endl;
        exit(-1);
    }
    return output;
}

void Function::clearRegisterNum() {
    registerNum = 0;
}

LocalVariable Function::getMathExpression(const shared_ptr<ASTNode>& node, vector<MathExpression>& output) {
    if (const shared_ptr<NumberNode> numberNode = dynamic_pointer_cast<NumberNode>(node)) {
        LocalVariable var = {Type(TypeType::INT), "I "+to_string(numberNode->value)};
        return var;
    }
    if (const shared_ptr<IdentifierNode> identifier_node = dynamic_pointer_cast<IdentifierNode>(node)) {
        LocalVariable local_variable = localVariableMap.at(identifier_node->name);
        return local_variable;
    }
    if (const shared_ptr<LogicalNode> log = dynamic_pointer_cast<LogicalNode>(node)) {
        output.push_back({getMathExpression(log->left, output), getMathExpression(log->right, output), log->logicalType});
        return {};
    }
    if (const shared_ptr<ArithmeticNode> ari = dynamic_pointer_cast<ArithmeticNode>(node)) {
        output.push_back({getMathExpression(ari->left, output), getMathExpression(ari->right, output), ari->arithmeticType});
        return {};
    }
    throw runtime_error("invalid logical expression AST Node");
}

string compile(const vector<shared_ptr<ASTNode>>& ast, const vector<FunctionDescr>& function_descrs, const unordered_map<string, unordered_map<string, Type>>& variables) {
    string output;
    output += "SEG\n";
    output += "MOVE W I H'00FFFF',SP\n";
    output += "MOVEA heap,HP\n";
    output += "CALL main\n";
    output += "HALT\n";
    for (int i = 0; i < ast.size(); i++) {
        shared_ptr<FunctionDefinitionNode> func = dynamic_pointer_cast<FunctionDefinitionNode>(ast[i]);
        Function function = Function(func, variables.at(func->functionName), function_descrs);
        output += function.getOutput();
    }
    generateMallocFunction(output);
    output += "HP: DD W 0\n";
    output += "heap: DD W 0\n";
    output += "END";
    return output;
}
