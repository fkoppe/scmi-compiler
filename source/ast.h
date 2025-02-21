#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>
#include <iostream>

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
        std::cout << std::string(indent, ' ') << "Number(" << value << ")\n";
    }
};

// AST-Knoten für Variablen (Identifier)
class IdentifierNode : public ASTNode {
public:
    std::string name;

    explicit IdentifierNode(std::string n) : name(std::move(n)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Identifier(" << name << ")\n";
    }
};

// AST-Knoten für Zuweisungen (e.g. `x = 5;`)
class AssignmentNode : public ASTNode {
public:
    std::shared_ptr<IdentifierNode> variable;
    std::shared_ptr<ASTNode> expression;

    AssignmentNode(std::shared_ptr<IdentifierNode> var, std::shared_ptr<ASTNode> expr)
        : variable(std::move(var)), expression(std::move(expr)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Assignment:\n";
        variable->print(indent + 2);
        expression->print(indent + 2);
    }
};

// AST-Knoten für Funktionsaufrufe (e.g. `myFunction(5, x);`)
class FunctionCallNode : public ASTNode {
public:
    std::string functionName;
    std::vector<std::shared_ptr<ASTNode>> arguments;

    explicit FunctionCallNode(std::string name) : functionName(std::move(name)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "FunctionCall(" << functionName << ")\n";
        for (const auto& arg : arguments) {
            arg->print(indent + 2);
        }
    }
};

class FunctionDefinitionNode : public ASTNode {
public:
    std::string returnType;
    std::string functionName;
    std::vector<std::pair<std::string, std::string>> parameters;
    std::vector<std::shared_ptr<ASTNode>> body;

    FunctionDefinitionNode(std::string rType, std::string fName,
                           std::vector<std::pair<std::string, std::string>> params,
                           std::vector<std::shared_ptr<ASTNode>> b)
        : returnType(std::move(rType)), functionName(std::move(fName)),
        parameters(std::move(params)), body(std::move(b)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "FunctionDefinition(" << functionName << ")\n";
        for (const auto& stmt : body) {
            stmt->print(indent + 2);
        }
    }
};

class ArrayDeclarationNode : public ASTNode {
public:
    std::string identifier;
    std::string elementType;
    int size;
    std::string name;
    std::vector<std::shared_ptr<ASTNode>> values;

    ArrayDeclarationNode(std::string id, std::string type, int s, std::string n, std::vector<std::shared_ptr<ASTNode>> vals)
        : identifier(std::move(id)), elementType(std::move(type)), size(s), name(std::move(n)), values(std::move(vals)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "ArrayDeclaration(" << identifier << ")\n";
    }
};

class ArrayAssignmentNode : public ASTNode {
public:
    std::shared_ptr<IdentifierNode> arrayName;
    std::shared_ptr<ASTNode> index;
    std::shared_ptr<ASTNode> value;

    ArrayAssignmentNode(std::shared_ptr<IdentifierNode> name, std::shared_ptr<ASTNode> idx, std::shared_ptr<ASTNode> val)
        : arrayName(std::move(name)), index(std::move(idx)), value(std::move(val)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "ArrayAssignment:\n";
        std::cout << std::string(indent + 2, ' ') << "Array: " << arrayName->name << "\n";
        std::cout << std::string(indent + 2, ' ') << "Index:\n";
        index->print(indent + 4);
        std::cout << std::string(indent + 2, ' ') << "Value:\n";
        value->print(indent + 4);
    }
};

class VariableDeclarationNode : public ASTNode {
public:
    std::string varType;
    std::string varName;
    std::shared_ptr<ASTNode> value;

    VariableDeclarationNode(std::string type, std::string name, std::shared_ptr<ASTNode> val)
        : varType(std::move(type)), varName(std::move(name)), value(std::move(val)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "VariableDeclaration(" << varType << " " << varName << ")\n";
        value->print(indent + 2);
    }
};



#endif // AST_H
