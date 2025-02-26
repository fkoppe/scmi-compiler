#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>
#include <iostream>

#include "Keyword.hpp"

using namespace std;

// AST Basisklasse
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
};

// AST-Knoten für Zahlen
class NumberNode : public ASTNode {
public:
    int value;

    explicit NumberNode(int val) : value(val) {}

    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "Number(" << value << ")\n";
    }
};

// AST-Knoten für Variablen (Identifier)
class IdentifierNode : public ASTNode {
public:
    string name;

    explicit IdentifierNode(string n) : name(move(n)) {}

    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "Identifier(" << name << ")\n";
    }
};

// AST-Knoten für Zuweisungen (e.g. `x = 5;`)
class AssignmentNode : public ASTNode {
public:
    shared_ptr<IdentifierNode> variable;
    shared_ptr<ASTNode> expression;

    AssignmentNode(shared_ptr<IdentifierNode> var, shared_ptr<ASTNode> expr)
        : variable(move(var)), expression(move(expr)) {}

    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "Assignment:\n";
        variable->print(indent + 2);
        expression->print(indent + 2);
    }
};

// AST-Knoten für Funktionsaufrufe (e.g. `myFunction(5, x);`)
class FunctionCallNode : public ASTNode {
public:
    string functionName;
    vector<shared_ptr<ASTNode>> arguments;

    explicit FunctionCallNode(string name) : functionName(move(name)) {}

    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "FunctionCall(" << functionName << ")\n";
        for (const auto& arg : arguments) {
            arg->print(indent + 2);
        }
    }
};

// AST-Knoten für return
class ReturnNode : public ASTNode {
public:
    explicit ReturnNode() {}

    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "Return\n";
    }
};

// AST-Knoten für return
class ReturnValueNode : public ASTNode {
public:
    shared_ptr<ASTNode> value;

    explicit ReturnValueNode(shared_ptr<ASTNode> val) : value(val) {}

    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "ReturnValue\n";
        value->print(indent + 2);
    }
};

class FunctionDefinitionNode : public ASTNode {
public:
    Type returnType;
    string functionName;
    vector<pair<Type, string>> parameters;
    vector<shared_ptr<ASTNode>> body;

    FunctionDefinitionNode(Type rType, string fName,
                           vector<pair<Type, string>> params,
                           vector<shared_ptr<ASTNode>> b)
        : returnType(move(rType)), functionName(move(fName)),
        parameters(move(params)), body(move(b)) {}

    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "FunctionDefinition(" << functionName << ")\n";
        for (const auto& stmt : body) {
            stmt->print(indent + 2);
        }
    }
};

class ArrayDeclarationNode : public ASTNode {
public:
    string identifier;
    Type elementType;
    int size;
    string name;
    vector<shared_ptr<ASTNode>> values;

    ArrayDeclarationNode(string id, Type type, int s, string n, vector<shared_ptr<ASTNode>> vals)
        : identifier(move(id)), elementType(move(type)), size(s), name(move(n)), values(move(vals)) {}

    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "ArrayDeclaration(" << identifier << ")\n";
    }
};

class ArrayAssignmentNode : public ASTNode {
public:
    shared_ptr<IdentifierNode> arrayName;
    shared_ptr<ASTNode> index;
    shared_ptr<ASTNode> value;

    ArrayAssignmentNode(shared_ptr<IdentifierNode> name, shared_ptr<ASTNode> idx, shared_ptr<ASTNode> val)
        : arrayName(move(name)), index(move(idx)), value(move(val)) {}

    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "ArrayAssignment:\n";
        cout << string(indent + 2, ' ') << "Array: " << arrayName->name << "\n";
        cout << string(indent + 2, ' ') << "Index:\n";
        index->print(indent + 4);
        cout << string(indent + 2, ' ') << "Value:\n";
        value->print(indent + 4);
    }
};

class VariableDeclarationNode : public ASTNode {
public:
    Type varType;
    string varName;
    shared_ptr<ASTNode> value;

    VariableDeclarationNode(Type type, string name, shared_ptr<ASTNode> val)
        : varType(move(type)), varName(move(name)), value(move(val)) {}

    void print(int indent = 0) const override {
        cout << string(indent, ' ') << "VariableDeclaration(" << varType.toString() << " " << varName << ")\n";
        value->print(indent + 2);
    }
};

// AST Node for `if` statements
class IfNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> condition;
    std::vector<std::shared_ptr<ASTNode>> thenBlock;
    std::vector<std::shared_ptr<ASTNode>> elseBlock;

    IfNode(std::shared_ptr<ASTNode> cond,
           std::vector<std::shared_ptr<ASTNode>> thenBlk,
           std::vector<std::shared_ptr<ASTNode>> elseBlk = {})
        : condition(std::move(cond)), thenBlock(std::move(thenBlk)), elseBlock(std::move(elseBlk)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "IfStatement\n";

        std::cout << std::string(indent + 2, ' ') << "Condition\n";
        condition->print(indent + 4);

        std::cout << std::string(indent + 2, ' ') << "Then Block\n";
        for (const auto& stmt : thenBlock) {
            stmt->print(indent + 4);
        }

        if (!elseBlock.empty()) {
            std::cout << std::string(indent + 2, ' ') << "Else Block\n";
            for (const auto& stmt : elseBlock) {
                stmt->print(indent + 4);
            }
        }
    }
};


#endif // AST_H
