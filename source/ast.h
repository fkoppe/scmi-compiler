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

// AST-Knoten für Variablen (Identifier), wenn size != -1 dann Array Indizierung
class IdentifierNode : public ASTNode {
public:
    string name;
    int index;

    explicit IdentifierNode(string n) : name(move(n)), index(-1) {}
    explicit IdentifierNode(string n, int index) : name(move(n)), index(index) {}


    void print(int indent = 0) const override {
        if (index == -1) {
            cout << string(indent, ' ') << "Identifier(" << name << ")\n";
        }
        else {
            cout << string(indent, ' ') << "Identifier(" << name<<"["<<index << "])\n";
        }
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

// Enum for logical operators
enum class LogicalType {
    AND,  // &&
    OR,    // ||
    EQUAL,          // ==
    NOT_EQUAL,      // !=
    LESS_THAN,      // <
    GREATER_THAN,   // >
    LESS_EQUAL,     // <=
    GREATER_EQUAL   // >=
};

// AST Node for logical expressions (x && y, a || b)
class LogicalNode : public ASTNode {
public:
    LogicalType logicalType;
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;

    LogicalNode(LogicalType type, std::shared_ptr<ASTNode> lhs, std::shared_ptr<ASTNode> rhs)
        : logicalType(type), left(std::move(lhs)), right(std::move(rhs)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "LogicalExpression(" << getLogicalOperator() << ")\n";
        left->print(indent + 2);
        right->print(indent + 2);
    }

private:
    std::string getLogicalOperator() const {
        switch (logicalType) {
        case LogicalType::AND: return "&&";
        case LogicalType::OR: return "||";
        case LogicalType::EQUAL: return "==";
        case LogicalType::NOT_EQUAL: return "!=";
        case LogicalType::LESS_THAN: return "<";
        case LogicalType::GREATER_THAN: return ">";
        case LogicalType::LESS_EQUAL: return "<=";
        case LogicalType::GREATER_EQUAL: return ">=";
        }
        return "UNKNOWN";
    }
};

// AST Node for logical NOT (!expr)
class LogicalNotNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> operand;

    explicit LogicalNotNode(std::shared_ptr<ASTNode> expr)
        : operand(std::move(expr)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "LogicalNotExpression(!)\n";
        operand->print(indent + 2);
    }
};

// Enum for arithmetic operators
enum class ArithmeticType {
    ADD,       // +
    SUBTRACT,  // -
    MULTIPLY,  // *
    DIVIDE,    // /
    MODULO     // %
};

// AST Node for arithmetic expressions (e.g., x + y, a * b)
class ArithmeticNode : public ASTNode {
public:
    ArithmeticType arithmeticType;
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;

    ArithmeticNode(ArithmeticType type, std::shared_ptr<ASTNode> lhs, std::shared_ptr<ASTNode> rhs)
        : arithmeticType(type), left(std::move(lhs)), right(std::move(rhs)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "ArithmeticExpression(" << getOperator() << ")\n";
        left->print(indent + 2);
        right->print(indent + 2);
    }

private:
    std::string getOperator() const {
        switch (arithmeticType) {
        case ArithmeticType::ADD: return "+";
        case ArithmeticType::SUBTRACT: return "-";
        case ArithmeticType::MULTIPLY: return "*";
        case ArithmeticType::DIVIDE: return "/";
        case ArithmeticType::MODULO: return "%";
        }
        return "UNKNOWN";
    }
};

class ArrayDeclarationNode : public ASTNode {
public:
    Type type;
    int32_t size;
    vector<shared_ptr<ASTNode>> arrayValues;
    string name;


    ArrayDeclarationNode(Type type, int32_t size, vector<shared_ptr<ASTNode>> arrayValues, string name )
        : type(type), size(size), arrayValues(std::move(arrayValues)), name(std::move(name)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "ArrayDeclarationNode(" << type.toString() << " "<< name << ")\n";
        if (arrayValues.size() > 0) {
            for (const auto& value : arrayValues) {
                value->print(indent + 2);
            }
        }
        else {
            cout << std::string(indent + 2, ' ') << "Empty("<< size <<")\n";
        }
    }
};

// AST Node for Goto Statement
class GotoNode : public ASTNode {
public:
    std::string label;
    explicit GotoNode(std::string lbl) : label(std::move(lbl)) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Goto(" << label << ")\n";
    }
};

class LabelNode : public ASTNode {
public:
    std::string label;

    explicit LabelNode(std::string lbl) : label(std::move(lbl)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Label(" << label << ")\n";
    }
};

// AST Node for While Loop
class WhileNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> condition;
    std::vector<std::shared_ptr<ASTNode>> body;
    WhileNode(std::shared_ptr<ASTNode> cond, std::vector<std::shared_ptr<ASTNode>> b)
        : condition(std::move(cond)), body(std::move(b)) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "WhileLoop\n";
        condition->print(indent + 2);
        for (const auto &stmt : body) {
            stmt->print(indent + 2);
        }
    }
};

// AST Node for For Loop
class ForNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> init;
    std::shared_ptr<ASTNode> condition;
    std::shared_ptr<ASTNode> update;
    std::vector<std::shared_ptr<ASTNode>> body;
    ForNode(std::shared_ptr<ASTNode> i, std::shared_ptr<ASTNode> cond, std::shared_ptr<ASTNode> upd, std::vector<std::shared_ptr<ASTNode>> b)
        : init(std::move(i)), condition(std::move(cond)), update(std::move(upd)), body(std::move(b)) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "ForLoop\n";
        init->print(indent + 2);
        condition->print(indent + 2);
        update->print(indent + 2);
        for (const auto &stmt : body) {
            stmt->print(indent + 2);
        }
    }
};

// AST Node for For Loop
class BlockNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> body;
    BlockNode(std::vector<std::shared_ptr<ASTNode>> b)
        : body(std::move(b)) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Block\n";
        for (const auto &stmt : body) {
            stmt->print(indent + 2);
        }
    }
};

#endif // AST_H
