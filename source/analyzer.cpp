#include "analyzer.hpp"

#include <unordered_set>
#include <utility>

/**
 * Starts the analysis process on the given AST nodes.
 *
 * @param nodes An array of AST nodes, expected to contain function definitions.
 * @return A pair consisting of:
 *         - An array of FunctionDescriptions representing the analyzed functions.
 *         - A map: mapping function names (strings) to their respective
 *           local variable maps, which map variable names (strings) to their types (Type).
 */
pair<vector<FunctionDescr>,unordered_map<string,unordered_map<string,Type>>> analyze(vector<shared_ptr<ASTNode>>& nodes) {
    vector<FunctionDescr> function_descrs;
    unordered_map<string, unordered_map<string,Type>> mapVariableList;

    vector<shared_ptr<FunctionDefinitionNode>> functions;
    unordered_set<string> labelNames;

    for (const shared_ptr<ASTNode>& ast : nodes) {
        // Check if the node is a function definition
        if (const shared_ptr<FunctionDefinitionNode> func = dynamic_pointer_cast<FunctionDefinitionNode>(ast)) {
            functions.push_back(func);

            // Extract function parameters and store them as (name, type) pairs
            vector<pair<string,Type>> paramVariables;
            for (const pair<Type,string>& p: func->parameters) {
                paramVariables.push_back({p.second, {p.first}});
            }

            function_descrs.push_back({func->functionName, {func->returnType}, paramVariables});

            // Iterate over the function body to check for label definitions
            for (const auto& x: func->body) {
                if (auto e = dynamic_pointer_cast<LabelNode>(x)) {
                    string labelName = e->label;

                    if (labelNames.count(labelName)) {
                        cout << "Label " << labelName << " already exists" << endl;
                        exit(-1);
                    }

                    checkGotoLabelName(labelName);

                    labelNames.insert(labelName);
                }
            }

        }
        else {
            cout << "Function declaration in AST Node not found" << endl;
        }
    }

    // Validate function names (ensures uniqueness and checks if 'main' exists)
    checkFunctionNames(function_descrs);

    // Perform semantic analysis on each function with the appropriate variable maps
    for (shared_ptr<FunctionDefinitionNode> &node : functions) {
        SemanticAnalyzer analyzer = SemanticAnalyzer(node, function_descrs, labelNames);
        mapVariableList.insert_or_assign(analyzer.getName(),analyzer.getVariableList());
    }

    return {function_descrs, mapVariableList};
}

//Constructor of SemanticAnalyzer
SemanticAnalyzer::SemanticAnalyzer(const shared_ptr<FunctionDefinitionNode>& function_node, const vector<FunctionDescr>& function_descrs, const unordered_set<string>& labelNames) {
    name = function_node->functionName;
    this->function_descrs = function_descrs;
    this->function_node = function_node;
    this->checkReturn = false;
    this->labelNames = labelNames;

    checkParams();

    for (const shared_ptr<ASTNode>& node : function_node->body) {
        checkNode(node, true);
    }

    // Check if the function has a non-void return type but lacks a return statement
    if (function_node->returnType.getEnum() != TypeType::VOID && !checkReturn) {
        cout << "function '" << name << "' ["<<function_node->returnType.toString()<<"] has no return" << endl;
        exit(-1);
    }
}

//Check all different types of ASTNodes in function body
void SemanticAnalyzer::checkNode(const shared_ptr<ASTNode>& node, bool declaration) {
    if (const shared_ptr<VariableDeclarationNode> var = dynamic_pointer_cast<VariableDeclarationNode>(node)) {
        if (!declaration) {
            cout << "Variable declarations are not allowed in ifStatement/Loop in function '"<< this->name << "'" <<endl;
            exit(-1);
        }

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
        checkReturn = true;
    }
    else if (const shared_ptr<ReturnNode>& return_node = dynamic_pointer_cast<ReturnNode>(node)) {
        if (function_node->returnType.getEnum() != TypeType::VOID) {
            cout << "return value ["<< function_node->returnType.toString() <<"] need to be specified in '"<< function_node->functionName <<"'" << endl;
            exit(-1);
        }
    }
    else if (const shared_ptr<IfNode> if_node = dynamic_pointer_cast<IfNode>(node)) {
        checkLogicalExpression(if_node->condition);

        for (const auto& x: if_node->thenBlock) {
            checkNode(x, false);
        }
        for (const auto& x: if_node->elseBlock) {
            checkNode(x, false);
        }
    }
    else if (const shared_ptr<WhileNode> while_node = dynamic_pointer_cast<WhileNode>(node)) {
        checkLogicalExpression(while_node->condition);

        for (const auto& x: while_node->body) {
            checkNode(x, false);
        }
    }
    else if (const shared_ptr<ForNode> for_node = dynamic_pointer_cast<ForNode>(node)) {
        if (auto x = dynamic_pointer_cast<VariableDeclarationNode>(for_node->init)) {
            checkDeclaration(x);
        }
        else {
            cout << "First parameter in for loop must be a Variable declaration" << endl;
            exit(-1);
        }

        if (auto x = dynamic_pointer_cast<LogicalNode>(for_node->condition)) {
            checkLogicalExpression(x);
        }
        else if (auto x = dynamic_pointer_cast<LogicalNotNode>(for_node->condition)) {
            checkLogicalExpression(x);
        }
        else {
            cout << "Second parameter in for loop must be a Logical Expression" << endl;
            exit(-1);
        }

        if (auto x = dynamic_pointer_cast<AssignmentNode>(for_node->update)) {
            checkAssignment(x);
        }
        else {
            cout << "Third parameter in for loop must be a Assignment" << endl;
            exit(-1);
        }

        for (const auto& x: for_node->body) {
            checkNode(x, false);
        }
    }
    else if (const shared_ptr<ArrayDeclarationNode> array = dynamic_pointer_cast<ArrayDeclarationNode>(node)) {
        checkDeclaration(make_shared<VariableDeclarationNode>(VariableDeclarationNode(array->type, array->name, nullptr)));

        Type arrayVarType = convertArrayToVarType(array->type);

        //check if array declaration is size zero
        if (array->size == 0 && array->arrayValues.size() == 0) {
            cout << "Array '" << array->name << "' is empty" << endl;
            exit(-1);
        }

        //check type of each declared variable element expression
        for (auto x: array->arrayValues) {
            if (getVariableType(x, arrayVarType).getEnum() != arrayVarType.getEnum()) {
                cout << "invalid Number Type for Array declaration" << endl;
                exit(-1);
            }
        }
    }
    else if (const shared_ptr<GotoNode> goto_node = dynamic_pointer_cast<GotoNode>(node)) {
        if (!this->labelNames.count(goto_node->label)) {
            cout << "Goto '" << goto_node->label << "' does not exist" << endl;
            exit(-1);
        }
    }
}

//Determines the type of given ASTNode while ensuring it conforms to the expected type.
Type SemanticAnalyzer::getVariableType(const shared_ptr<ASTNode>& node, const Type& expected_type) {
    if (shared_ptr<IdentifierNode> ident = dynamic_pointer_cast<IdentifierNode>(node)){
        checkIdentifier(ident);
        Type foundType = findVariable(ident->name);

        if (ident->index != nullptr) {
            checkIndex(ident->index);
            foundType = convertArrayToVarType(foundType);
        }

        return getCastType(foundType, expected_type);
    }
    if (shared_ptr<FunctionCallNode> function_call = dynamic_pointer_cast<FunctionCallNode>(node)){
        FunctionDescr call_func = checkFunctionCall(function_call);
        return getCastType(call_func.type, expected_type);
    }

    //checks NumberNode in respect to the type value limits
    if (shared_ptr<NumberNode> number_node = dynamic_pointer_cast<NumberNode>(node)) {
        if (expected_type.getEnum() == TypeType::INT && number_node->value <= maxInt && number_node->value >= minInt) {
            return Type(TypeType::INT);
        }
        if (expected_type.getEnum() == TypeType::SHORT && number_node->value <= maxShort && number_node->value >= minShort) {
            return Type(TypeType::SHORT);
        }
        if (expected_type.getEnum() == TypeType::CHAR && number_node->value <= maxChar && number_node->value >= minChar) {
            return Type(TypeType::CHAR);
        }
        cout << "Invalid number: " << number_node->value << " for type '" << expected_type.toString() << "' in function '" << this->name << "'" << endl;
        exit(-1);
    }

    //Logical Nodes are always INT
    if (const shared_ptr<LogicalNode> logical_node = dynamic_pointer_cast<LogicalNode>(node)) {
        checkLogicalExpression(logical_node);
        return Type(TypeType::INT);
    }
    if (const shared_ptr<ArithmeticNode> arithmetic_node = dynamic_pointer_cast<ArithmeticNode>(node)) {
        return getArithmeticType(arithmetic_node, expected_type);
    }
    throw runtime_error("Unrecognized node type for getVariableType");
}

//same to getVariableType but with no return and expected Type
void SemanticAnalyzer::checkExpression(const shared_ptr<ASTNode>& node) {
    if (shared_ptr<IdentifierNode> ident = dynamic_pointer_cast<IdentifierNode>(node)){
        checkIdentifier(ident);
        if (ident->index != nullptr) {
            this->checkIndex(ident->index);
        }
    }
    else if (shared_ptr<FunctionCallNode> function_call = dynamic_pointer_cast<FunctionCallNode>(node)){
        FunctionDescr call_func = checkFunctionCall(function_call);
    }
    else if (shared_ptr<NumberNode> number_node = dynamic_pointer_cast<NumberNode>(node)) {
        if (number_node->value < maxInt && number_node->value > minInt) {

        }
        else if (number_node->value < maxShort && number_node->value > minShort) {

        }
        else if (number_node->value < maxChar && number_node->value > minChar) {

        }
        else {
            cout << "Invalid number: " << number_node->value << endl;
            exit(-1);
        }
    }
    else if (shared_ptr<ArithmeticNode> arithmetic_node = dynamic_pointer_cast<ArithmeticNode>(node)) {

    }
    else {
        throw runtime_error("Unrecognized node type for checkExpression");
    }
}

void SemanticAnalyzer::checkIndex(const shared_ptr<ASTNode>& index) {
    if (getVariableType(index, Type(TypeType::INT)).getEnum() != TypeType::INT) {
        cout << "invalid type for index" << endl;
        exit(-1);
    }
}

FunctionDescr SemanticAnalyzer::checkFunctionCall(const shared_ptr<FunctionCallNode>& function_call_node) {
    FunctionDescr call_func;
    bool found = false;

    //find function description and set bool
    for (const FunctionDescr& x: function_descrs) {
        if (function_call_node->functionName == x.name) {
            found = true;
            call_func = x;
            break;
        }
    }

    //Handle output function (@output(x...))
    if (function_call_node->functionName == OUTPUT_FUNCTION && function_call_node->arguments.size() <= 13) {
        found = true;
        vector<pair<string,Type>> params;

        if (function_call_node->arguments.size() != 1) {
            cout << "Invalid number of arguments for " << OUTPUT_FUNCTION << endl;
            exit(-1);
        }

        shared_ptr<ASTNode> argument = function_call_node->arguments.at(0);

        Type type = getVariableType(argument, Type(TypeType::INT));
        if (type.getEnum() != TypeType::INT) {
            cout << "invalid type for argument in " << OUTPUT_FUNCTION << endl;
            exit(-1);
        }

        params.emplace_back("", type);

        call_func = {function_call_node->functionName, Type(TypeType::VOID), params};
    }

    if (function_call_node->functionName == LENGTH_FUNCTION) {
        found = true;
        vector<pair<string,Type>> params;

        if (function_call_node->arguments.size() > 1) {
            cout << "Only one parameter allowed in @length function" << endl;
            exit(-1);
        }

        auto node = function_call_node->arguments.at(0);

        if (auto x = dynamic_pointer_cast<IdentifierNode>(node)) {
            if (!variableList.at(x->name).isArray()) {
                cout << "Parameter in @length function must be type array" << endl;
                exit(-1);
            }

            params.emplace_back("",variableList.at(x->name));
        }
        else {
            cout << "Parameter in @length function must be type array" << endl;
            exit(-1);
        }

        call_func = {function_call_node->functionName, Type(TypeType::INT), params};
    }


    if (!found) {
        cout << "Function call '" << function_call_node->functionName << "' in '" << this->name << "' not found" << endl;
        exit(-1);
    }

    //check params
    if (function_call_node->arguments.size() != call_func.params.size()) {
        cout << "Function call '" << function_call_node->functionName << "' in '" << this->name << "' does not have the correct number of arguments" << endl;
        exit(-1);
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

    Type definition = convertArrayToVarType(findVariable(ass->variable->name));
    Type input = getVariableType(ass->expression, definition);
    checkIdentifierType(ass->variable->name, definition, "<assignment>", input);
}

void SemanticAnalyzer::checkParams() {
    FunctionDescr ownDescr = findFunctionDescr(this->name);

    for (const auto&[name, type]: ownDescr.params) {
        checkForbiddenIdentifier(name);
        if (!variableList.try_emplace(name, type).second) {
            cout << "Parameter '" << name << "' for function '"<< this->name << "' already exists" << endl;
        }
    }
}

void SemanticAnalyzer::checkLogicalExpression(const shared_ptr<ASTNode>& condition_node) {
    if (const shared_ptr<LogicalNode> logical = dynamic_pointer_cast<LogicalNode>(condition_node)) {
        checkLogicalExpression(logical->left);
        checkLogicalExpression(logical->right);
    }
    else if (const shared_ptr<LogicalNotNode> logical_not_node = dynamic_pointer_cast<LogicalNotNode>(condition_node)) {
        checkLogicalExpression(logical_not_node->operand);
    }
    else {
        checkExpression(condition_node);
    }
}

/**
 * Recursively checks that all elements in an ArithmeticNode structure match the expected type.
 */
Type SemanticAnalyzer::getArithmeticType(const shared_ptr<ArithmeticNode>& arithmetic_node, const Type& expected) {
    Type left = getVariableType(arithmetic_node->left, expected);
    Type right = getVariableType(arithmetic_node->right, expected);

    if (left.getEnum() == right.getEnum()) {
        return left;
    }
    cout << "Arithmetic Expression has not the same type [" << expected.toString() << "] in function '" << this->name << "'" << endl;
    exit(-1);
}

/**
 * Checks VariableDeclarationNodes:
 * - Ensures the variable name is not a forbidden identifier.
 * - Attempts to insert the variable into the variable list.
 * - If the variable is already declared in the current function scope, an error is printed, and the program exits.
 */
void SemanticAnalyzer::checkDeclaration(const shared_ptr<VariableDeclarationNode> &var) {
    checkForbiddenIdentifier(var->varName);

    Type type = {var->varType};
    if (!variableList.try_emplace(var->varName, type).second) {
        cout << "Variable '" << var->varName << "' in function '" << name << "' is already declared" << endl;
        exit(-1);
    }
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

//casts "lower" type to "higher" type
Type SemanticAnalyzer::getCastType(Type found, Type expected) {
    int foundNum = static_cast<int>(found.getEnum());
    int expectedNum = static_cast<int>(expected.getEnum());

    if (foundNum <= expectedNum) {
        return expected;
    }
    return found;
}

unordered_map<string, Type> SemanticAnalyzer::getVariableList() {
    return variableList;
}

string SemanticAnalyzer::getName() {
    return name;
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

void checkGotoLabelName(const string &name) {
    if (name[0] == '_' && name[1] == '_' && name[name.length()-1] == '_' && name[name.length()-2] == '_') {
        cout << "Invalid label name: " << name << endl;
        exit(-1);
    }
}
