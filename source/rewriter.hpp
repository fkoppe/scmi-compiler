#ifndef REWRITER_H
#define REWRITER_H

#include "ast.h"
#include <iostream>

class Rewriter {
public:
    std::shared_ptr<ASTNode> optimize(std::shared_ptr<ASTNode> node) {
        if (!node) return nullptr;

        // Optimize specific node types
        if (auto assign = dynamic_pointer_cast<AssignmentNode>(node)) {
            return optimizeAssignment(assign);
        } else if (auto arith = dynamic_pointer_cast<ArithmeticNode>(node)) {
            return optimizeArithmetic(arith);
        } else if (auto logical = dynamic_pointer_cast<LogicalNode>(node)) {
            return optimizeLogical(logical);
        } else if (auto ifNode = dynamic_pointer_cast<IfNode>(node)) {
            return optimizeIf(ifNode);
        } else if (auto funcCall = dynamic_pointer_cast<FunctionCallNode>(node)) {
            return optimizeFunctionCall(funcCall);
        } else if (auto funcDef = dynamic_pointer_cast<FunctionDefinitionNode>(node)) {
            return optimizeFunctionDefinition(funcDef);
        }
        return node;
    }

private:
    std::shared_ptr<ASTNode> optimizeAssignment(std::shared_ptr<AssignmentNode> node) {
        node->expression = optimize(node->expression);
        if (auto id = dynamic_pointer_cast<IdentifierNode>(node->expression)) {
            if (id->name == node->variable->name) {
                std::cout << "Removed self-assignment\n";
                return nullptr;
            }
        }
        return node;
    }

    std::shared_ptr<ASTNode> optimizeArithmetic(std::shared_ptr<ArithmeticNode> node) {
        node->left = optimize(node->left);
        node->right = optimize(node->right);

        if (auto leftNum = dynamic_pointer_cast<NumberNode>(node->left)) {
            if (auto rightNum = dynamic_pointer_cast<NumberNode>(node->right)) {
                int result = 0;
                switch (node->arithmeticType) {
                case ArithmeticType::ADD: result = leftNum->value + rightNum->value; break;
                case ArithmeticType::SUBTRACT: result = leftNum->value - rightNum->value; break;
                case ArithmeticType::MULTIPLY: result = leftNum->value * rightNum->value; break;
                case ArithmeticType::DIVIDE: if (rightNum->value != 0) result = leftNum->value / rightNum->value; break;
                case ArithmeticType::MODULO: if (rightNum->value != 0) result = leftNum->value % rightNum->value; break;
                }
                std::cout << "Removed comptime arithmetic\n";
                return std::make_shared<NumberNode>(result);
            }
        }
        return node;
    }

    std::shared_ptr<ASTNode> optimizeLogical(std::shared_ptr<LogicalNode> node) {
        node->left = optimize(node->left);
        node->right = optimize(node->right);

        if (auto leftNum = dynamic_pointer_cast<NumberNode>(node->left)) {
            if (auto rightNum = dynamic_pointer_cast<NumberNode>(node->right)) {
                bool result = false;
                switch (node->logicalType) {
                case LogicalType::AND: result = leftNum->value && rightNum->value; break;
                case LogicalType::OR: result = leftNum->value || rightNum->value; break;
                case LogicalType::EQUAL: result = leftNum->value == rightNum->value; break;
                case LogicalType::NOT_EQUAL: result = leftNum->value != rightNum->value; break;
                case LogicalType::LESS_THAN: result = leftNum->value < rightNum->value; break;
                case LogicalType::GREATER_THAN: result = leftNum->value > rightNum->value; break;
                case LogicalType::LESS_EQUAL: result = leftNum->value <= rightNum->value; break;
                case LogicalType::GREATER_EQUAL: result = leftNum->value >= rightNum->value; break;
                }
                std::cout << "Removed comptime logic\n";
                return std::make_shared<NumberNode>(result ? 1 : 0);
            }
        }
        return node;
    }

    std::shared_ptr<ASTNode> optimizeIf(std::shared_ptr<IfNode> node) {
        node->condition = optimize(node->condition);
        for (auto &stmt : node->thenBlock) stmt = optimize(stmt);
        for (auto &stmt : node->elseBlock) stmt = optimize(stmt);
        return node;
    }

    std::shared_ptr<ASTNode> optimizeFunctionCall(std::shared_ptr<FunctionCallNode> node) {
        for (auto &arg : node->arguments) {
            arg = optimize(arg);
        }
        return node;
    }

    std::shared_ptr<ASTNode> optimizeFunctionDefinition(std::shared_ptr<FunctionDefinitionNode> node) {
        for (auto &stmt : node->body) {
            stmt = optimize(stmt);
        }
        return node;
    }
};

#endif // REWRITER_H
