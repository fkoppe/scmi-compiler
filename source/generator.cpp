//
// Created by cedric on 21.02.25.
//
#include "generator.hpp"

#include <bits/locale_facets_nonio.h>

#include "ast.h"
#include "analyzer.hpp"

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
    output += "FREE: DD W 0\n";
    output += "HP: DD W 0\n";
    output += "heap: DD W 0\n";
    output += "END";
    return output;
}


//Constructor for each Function generator
Function::Function(const shared_ptr<FunctionDefinitionNode>& functionNode, const unordered_map<string, Type>& variables, const vector<FunctionDescr>& function_descrs) {
    this->functionName = functionNode->functionName;
    this->function_descr_vector = function_descrs;
    this->function_descr_own = findFunctionDescr(functionNode);
    this->returnLabel = function_descr_own.address+"__return__";
    this->localVariablePointerOffset = 0;
    this->paramaterPointerOffset = 0;
    this->jumpLabelNum = 0;
    this->registerNum = 0;

    //epilog
    output += function_descr_own.address + ":\n";

    if (functionName != "main") {
        output += "PUSHR\n";
    }

    output += "MOVE W SP,R13\n";

    int declaredVariableSize = addVariables(variables);
    output += "SUB W I " + to_string(declaredVariableSize) + ",SP\n";

    generateNodes(functionNode->body);

    //prolog
    output += returnLabel+":\n";
    output += "MOVE W R13,SP\n";
    if (functionName != "main") {
        output += "POPR\n";
    }
    output += "RET\n\n";
}

//generate "block" of ASTNodes
void Function::generateNodes(const vector<shared_ptr<ASTNode>>& node) {
    for (const shared_ptr<ASTNode>& bodyElement: node) {
        if (shared_ptr<VariableDeclarationNode> variable_declaration_node = dynamic_pointer_cast<VariableDeclarationNode>(bodyElement)) {
            generateAssignment(localVariableMap.at(variable_declaration_node->varName), variable_declaration_node->value);
        }
        else if (shared_ptr<AssignmentNode> assignment_node = dynamic_pointer_cast<AssignmentNode>(bodyElement)) {
            generateAssignment(localVariableMap.at(assignment_node->variable->name), assignment_node->variable->index, assignment_node->expression);
        }
        else if (shared_ptr<FunctionCallNode> function_call_node = dynamic_pointer_cast<FunctionCallNode>(bodyElement)) {
            //treat special output function exclusively
            if (function_call_node->functionName == OUTPUT_FUNCTION) {
                generateOutputFunction(function_call_node);
                continue;
            }

            if (function_call_node->functionName == SREF_FUNCTION) {
                generateSREF(function_call_node);
                continue;
            }

            FunctionDescr function_descr = findFunctionDescr(function_call_node);
            generateFunctionCall(function_call_node, function_descr);
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
            string reg = getNextRegister();
            //push condition to stack
            generateAssignment({Type(TypeType::INT), reg},if_node->condition);

            output += "MOVE W "+reg+",-!SP\n";
            clearRegisterNum();

            //jump to then/else block
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
            //array value declaration
            if (arr->size == -1) {
                elementSize = arr->arrayValues.size();
            }
            else {
                elementSize = arr->size;
            }
            int arraySize = elementSize * arrayElementType.size();
            malloc(arraySize+ARRAY_DESCRIPTOR_SIZE, local_variable.address);

            output += "MOVE W I "+ to_string(elementSize)+",!("+local_variable.address+")\n";

            if (arr->size == -1) {
                //fill values
                for (int i = 0; i < arr->arrayValues.size(); i++) {
                    string reg = generateArrayIndex(local_variable, make_shared<NumberNode>(i));
                    generateAssignment({arrayElementType,reg}, arr->arrayValues.at(i));
                    clearRegisterNum();
                }
            }
        }
        else if (const shared_ptr<LabelNode> label_node = dynamic_pointer_cast<LabelNode>(bodyElement)) {
            output += "__"+label_node->label+":\n";
        }
        else if (const shared_ptr<GotoNode> goto_node = dynamic_pointer_cast<GotoNode>(bodyElement)) {
            output += "JUMP __"+goto_node->label+"\n";
        }
        else if (const shared_ptr<BlockNode> block_node = dynamic_pointer_cast<BlockNode>(bodyElement)) {
            generateNodes(block_node->body);
        }
    }
}

//                                                                      index: for array indexing
void Function::generateAssignment(const LocalVariable& assign_variable, shared_ptr<ASTNode> assign_variable_index, const shared_ptr<ASTNode>& node_expression) {
    string assignment;
    Type assignType;

    if (const shared_ptr<NumberNode> numberNode = dynamic_pointer_cast<NumberNode>(node_expression)) {
        assignment = "I " + to_string(numberNode->value);
        assignType = assign_variable.type;
    }
    else if (const shared_ptr<IdentifierNode> identifier_node = dynamic_pointer_cast<IdentifierNode>(node_expression)) {
        LocalVariable local_variable = localVariableMap.at(identifier_node->name);

        //"normal" variable
        if (identifier_node->index == nullptr) {
            assignment = local_variable.address;
            assignType = local_variable.type;
        }
        //array indexing
        else {
            assignment = generateArrayIndex(local_variable, identifier_node->index);
            assignType = convertArrayToVarType(local_variable.type);
        }
    }
    else if (const shared_ptr<FunctionCallNode> function_call_node = dynamic_pointer_cast<FunctionCallNode>(node_expression)) {
        if (function_call_node->functionName == LENGTH_FUNCTION) {
            shared_ptr<IdentifierNode> param1 = dynamic_pointer_cast<IdentifierNode>(function_call_node->arguments.at(0));
            assignment = localVariableMap.at(param1->name).address;
            assignment = "!("+assignment+")";
            assignType = Type(TypeType::INT);
        }
        else if (function_call_node->functionName == DREF_FUNCTION) {
            string dref_reg = getNextRegister();
            auto arg = function_call_node->arguments.at(0);

            generateAssignment({Type(TypeType::INT), dref_reg}, arg);

            assignment = "!"+dref_reg;
            assignType = assign_variable.type;

            clearRegisterNum();
        }
        else {
            FunctionDescr function_call_type = findFunctionDescr(function_call_node);
            generateFunctionCall(function_call_node, function_call_type);
            string outputRegister = getNextRegister();

            //pop function return to Rx
            // output += "MOVE " + function_call_type.type.miType() + " !SP+,"+outputRegister+"\n";
            // assignment = outputRegister;
            // assignType = function_call_type.type;
            // clearRegisterNum();

            assignment = "!SP+";
            assignType = function_call_type.type;
        }
    }
    else if (const shared_ptr<LogicalNode> logical_node = dynamic_pointer_cast<LogicalNode>(node_expression)) {
        generateMathExpression(node_expression, assign_variable.type);
        assignment = "!SP+";
        //LogicalExpression is always INT
        assignType = Type(TypeType::INT);
    }
    else if (const shared_ptr<LogicalNotNode>& logical_node = dynamic_pointer_cast<LogicalNotNode>(node_expression)) {
        generateMathExpression(node_expression, assign_variable.type);
        assignment = "!SP+";
        //LogicalExpression is always INT
        assignType = Type(TypeType::INT);
    }
    else if (const shared_ptr<ArithmeticNode>& arithmetic_node = dynamic_pointer_cast<ArithmeticNode>(node_expression)) {
        //same like logical Node except Type
        generateMathExpression(node_expression, assign_variable.type);
        assignment = "!SP+";
        //type can be casted
        assignType = assign_variable.type;
    }
    else {
        throw runtime_error("invalid assignment AST Node");
    }

    string assignVariableAddress = getVariableAddress(assign_variable, assign_variable_index);

    if (convertArrayToVarType(assignType).getEnum() != convertArrayToVarType(assign_variable.type).getEnum()) {
         string shiftReg = getNextRegister();
         output += "MOVE " + assignType.miType() + " " + assignment + ","+shiftReg+"\n";
         output += "MOVE " + assignType.miType() + " " + shiftReg +  "," + assignVariableAddress + "\n";
         clearRegisterNum();
    }
    else {
        output += "MOVE " + assign_variable.type.miType() + " " + assignment + "," + assignVariableAddress + "\n";
    }


}


void Function::generateFunctionCall(const shared_ptr<FunctionCallNode>& function_call_node, const FunctionDescr& function_call_type) {
    //reserve output space
    int outputSize = function_call_type.type.size();
    if (outputSize != 0) {
        output += "SUB W I " + to_string(function_call_type.type.size()) + ",SP\n";
    }

    int inputSize = 0;

    //iterate backwards through params and push them on stack
    for (int i = function_call_type.params.size() - 1; i >= 0; i--) {
        Type paramType =  function_call_type.params.at(i).second;
        shared_ptr<ASTNode> arguments_node = function_call_node->arguments.at(i);
        string reg = getNextRegister();
        generateAssignment({paramType, reg}, arguments_node);
        output += "MOVE "+paramType.miType()+" "+reg+",-!SP\n";
        clearRegisterNum();
        inputSize += paramType.size();
    }

    output += "CALL " + function_call_type.address + "\n";

    //skip params in stack
    if (inputSize != 0) {
        output += "ADD W I " + to_string(inputSize) + ",SP\n";
    }
    //output is handled outside 'generateFunctionCall'
}

//wrapper for index=-1
void Function::generateAssignment(const LocalVariable &assign_variable, const shared_ptr<ASTNode> &node_expression) {
    generateAssignment(assign_variable,nullptr,node_expression);
}

//assign all variables addresses and store them in localVariableMap
int Function::addVariables(const unordered_map<string, Type>& variables) {
    localVariableMap["@HP"] = {Type(TypeType::INT), "HP"};
    localVariableMap["@FREE"] = {Type(TypeType::INT), "FREE"};

    unordered_set<string> params;
    params.reserve(function_descr_own.params.size());

    //param Variables
    int paramOffset = 0;
    for (const auto&[name, type]:function_descr_own.params) {
        params.insert(name);
        string address = to_string(64+paramOffset)+"+!R13";
        paramOffset += type.size();
        localVariableMap[name] = {type, address};
    }

    //add return variable
    localVariableMap["return"] = {function_descr_own.type,to_string(64+paramOffset)+"+!R13"};

    //local Variables
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

//special output Function to display values in register
void Function::generateOutputFunction(const shared_ptr<FunctionCallNode>& output) {
    auto argumment = output->arguments.at(0);
    //only outputs first paramter to R12
    generateAssignment({Type(TypeType::INT),"R12"},argumment);
}

void Function::generateLogicalExpression(const MathExpression& logical_expression) {
    //generate one operation post ordered elements (stack operations)

    LogicalType logType = get<LogicalType>(logical_expression.op);

    if (logical_expression.expression_L != nullptr) {
        string pushReg = getNextRegister();
        generateAssignment({Type(TypeType::INT),pushReg},logical_expression.expression_L);
        output += "MOVE W " + pushReg + ",-!SP\n";
        clearRegisterNum();
    }

    if (logical_expression.expression_R != nullptr) {
        string pushReg = getNextRegister();
        generateAssignment({Type(TypeType::INT),pushReg},logical_expression.expression_R);
        output += "MOVE W " + pushReg + ",-!SP\n";
        clearRegisterNum();
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
    else if (logType == LogicalType::NOT) {
        string trueLabel = getNextJumpLabel();
        string falseLabel = getNextJumpLabel();

        output += "CMP W I 0,!SP\n";
        output += "JEQ "+trueLabel+"\n";

        output += "MOVE W I 0,!SP\n";
        output += "JUMP "+falseLabel+"\n";

        output += trueLabel+":\n";
        output += "MOVE W I 1,!SP\n";

        output += falseLabel+":\n";
    }
    else {
        //==,!=,<,<=,>,>=
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
    string output = this->function_descr_own.address+"__jump__"+to_string(jumpLabelNum);
    jumpLabelNum++;
    return output;
}

void Function::generateArithmeticExpression(const MathExpression& arithmetic_expression, const Type& expected_type) {
    ArithmeticType ariType = get<ArithmeticType>(arithmetic_expression.op);

    if (arithmetic_expression.expression_L != nullptr) {
        string pushReg = getNextRegister();
        generateAssignment({expected_type,pushReg},arithmetic_expression.expression_L);
        output += "MOVE " + expected_type.miType() +" " + pushReg + ",-!SP\n";
        clearRegisterNum();
    }

    if (arithmetic_expression.expression_R != nullptr) {
        string pushReg = getNextRegister();
        generateAssignment({expected_type,pushReg},arithmetic_expression.expression_R);
        output += "MOVE " + expected_type.miType() +" " + pushReg + ",-!SP\n";
        clearRegisterNum();
    }

    generateArithmeticOperation(ariType, expected_type);
}

void Function::generateArithmeticOperation(const ArithmeticType arithmetic, const Type type) {
    if (arithmetic == ArithmeticType::MODULO) {
        string reg = getNextRegister();
        output += "DIV "+type.miType()+" !SP,4+!SP,"+reg+"\n"; //temp = a div b
        output += "MULT "+type.miType()+" !SP,"+reg+"\n"; // temp = b mult temp
        output += "SUB "+type.miType()+" "+reg+",4+!SP\n"; // erg = a -temp
        output += "ADD W I 4,SP\n";
        clearRegisterNum();
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
    output += op +" "+type.miType()+" !SP,4+!SP\n";
    output += "ADD W I 4,SP\n";
}

void Function::malloc(int size, const string& assignment) {
    output += "MOVE W I 0,-!SP\n";
    output += "MOVE W I "+ to_string(size) + ",-!SP\n";
    output += "CALL malloc\n";
    output += "ADD W I 4, SP\n";
    output += "MOVE W !SP+,"+assignment+"\n";
}

//get address to element of array with index and return !Rx (value of indexed element)
string Function::generateArrayIndex(const LocalVariable& local_variable, shared_ptr<ASTNode> index) {
    string reg = getNextRegister();
    string address = local_variable.address;
    int arrayElementSize = convertArrayToVarType(local_variable.type).size();

    //output += "MOVE W I "+to_string(index)+","+reg+"\n";

    generateAssignment({Type(TypeType::INT),reg}, index);

    output += "MULT W I "+to_string(arrayElementSize)+","+reg+"\n";
    output += "ADD W "+address + ","+reg+"\n";
    output += "ADD W I "+to_string(ARRAY_DESCRIPTOR_SIZE)+","+reg+"\n";

    return "!"+reg;
}

string Function::getVariableAddress(const LocalVariable& local_variable, shared_ptr<ASTNode> index) {
    if (index == nullptr) {
        return local_variable.address;
    }
    else {
        return generateArrayIndex(local_variable, index);
    }
}

//generate post order array with recursive data structure
void Function::generateMathExpression(const shared_ptr<ASTNode>& node, Type type) {
    vector<MathExpression> logical_expressions;
    getMathExpression(node, logical_expressions);

    for (const MathExpression& arithmetic_expression : logical_expressions) {
        if (holds_alternative<LogicalType>(arithmetic_expression.op)) {
            generateLogicalExpression(arithmetic_expression);
        }
        if (holds_alternative<ArithmeticType>(arithmetic_expression.op)) {
            generateArithmeticExpression(arithmetic_expression, type);
        }
    }
}

void Function::generateSREF(shared_ptr<FunctionCallNode> function_call_node) {
    auto arg1 = function_call_node->arguments.at(0);
    auto arg2 = dynamic_pointer_cast<IdentifierNode>(function_call_node->arguments.at(1));

    Type type2 = convertArrayToVarType(localVariableMap.at(arg2->name).type);

    string arg1_reg = getNextRegister();
    generateAssignment({Type(TypeType::INT),arg1_reg}, arg1);

    generateAssignment({type2,"!"+arg1_reg}, arg2);
    clearRegisterNum();
}

Type Function::getType(shared_ptr<ASTNode> node) {
    if (dynamic_pointer_cast<NumberNode>(node)) {
        return Type(TypeType::INT);
    }
    if (auto x = dynamic_pointer_cast<IdentifierNode>(node)) {
        if (x->index == nullptr) {
            return localVariableMap.at(x->name).type;
        }
        return convertArrayToVarType(localVariableMap.at(x->name).type);
    }
    if (auto x = dynamic_pointer_cast<FunctionCallNode>(node)) {
        return findFunctionDescr(x).type;
    }
    if (auto x = dynamic_pointer_cast<ArithmeticNode>(node)) {
        return Type(TypeType::INT);
    }
    if (auto x = dynamic_pointer_cast<LogicalNode>(node)) {
        return Type(TypeType::INT);
    }
    if (auto x = dynamic_pointer_cast<LogicalNotNode>(node)) {
        return Type(TypeType::INT);
    }
    throw runtime_error("Invalid node type in 'getType()'");
}

//recursive function for post order array
shared_ptr<ASTNode> Function::getMathExpression(const shared_ptr<ASTNode>& node, vector<MathExpression>& output) {
    if (const shared_ptr<LogicalNode> log = dynamic_pointer_cast<LogicalNode>(node)) {
        output.push_back({getMathExpression(log->left, output), getMathExpression(log->right, output), log->logicalType});
        return nullptr;
    }
    if (const shared_ptr<ArithmeticNode> ari = dynamic_pointer_cast<ArithmeticNode>(node)) {
        output.push_back({getMathExpression(ari->left, output), getMathExpression(ari->right, output), ari->arithmeticType});
        return nullptr;
    }
    if (const shared_ptr<LogicalNotNode> logNot = dynamic_pointer_cast<LogicalNotNode>(node)) {
        output.push_back({getMathExpression(logNot->operand, output), nullptr,LogicalType::NOT});
        return nullptr;
    }
    return node;
}

FunctionDescr Function::findFunctionDescr(shared_ptr<FunctionDefinitionNode> node) {
    for (auto x: function_descr_vector) {
        if (x.name == node->functionName) {
            if (x.params.size() == node->parameters.size()) {
                bool same = true;
                for (int i = 0; i < x.params.size(); i++) {
                    Type type1 = x.params.at(i).second;
                    Type type2 = node->parameters.at(i).first;
                    if (type1.getEnum() != type2.getEnum()) {
                        same = false;
                    }
                }
                if (same) {
                    return x;
                }
            }
        }
    }

    throw runtime_error("cannot find function: " + node->functionName);
}

FunctionDescr Function::findFunctionDescr(shared_ptr<FunctionCallNode> node) {
    for (auto x: function_descr_vector) {
        if (x.name == node->functionName) {
            if (x.params.size() == node->arguments.size()) {
                bool same = true;
                for (int i = 0; i < x.params.size(); i++) {
                    Type type1 = x.params.at(i).second;
                    Type type2 = getType(node->arguments.at(i));


                    if (type1.getEnum() != type2.getEnum()) {
                        same = false;
                    }
                    if (type1.getEnum() == TypeType::INT && type2.isArray()) {
                        same = true;
                    }
                }
                if (same) {
                    return x;
                }
            }
        }
    }

    throw runtime_error("cannot find function: " + node->functionName);
}





string Function::getNextRegister() {
    if (registerNum < 0) {
        throw runtime_error("register underflow");
    }
    string output = "R"+to_string(registerNum);
    registerNum++;
    //R15: PC, R14: SP, R13: BP, R12: OutputReg
    if (registerNum > 11) {
        throw runtime_error("register overflow");
    }
    return output;
}

void Function::clearRegisterNum() {
    registerNum--;
}

string Function::getOutput() {
    return output;
}

