#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>
#include <iostream>

// AST Basisklasse
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0; // Debug-Ausgabe des AST
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

#endif // AST_H
