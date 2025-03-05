#ifndef REWRITER_H
#define REWRITER_H

#include "ast.h"

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
        } else if (auto retVal = dynamic_pointer_cast<ReturnValueNode>(node)) {
            return optimizeReturnValue(retVal);
        }
        return node;
    }

private:
    std::shared_ptr<ASTNode> optimizeAssignment(std::shared_ptr<AssignmentNode> node) {
        if (auto id = dynamic_pointer_cast<IdentifierNode>(node->expression)) {
            if (id->name == node->variable->name) {
                return nullptr; // Remove self-assignment
            }
        }
        node->expression = optimize(node->expression);
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
                return std::make_shared<NumberNode>(result ? 1 : 0);
            }
        }
        return node;
    }

    std::shared_ptr<ASTNode> optimizeIf(std::shared_ptr<IfNode> node) {
        node->condition = optimize(node->condition);
        for (auto &stmt : node->thenBlock) stmt = optimize(stmt);
        for (auto &stmt : node->elseBlock) stmt = optimize(stmt);

        if (auto condNum = dynamic_pointer_cast<NumberNode>(node->condition)) {
            //if (condNum->value) {
            //    return std::make_shared<ASTNode>(node->thenBlock);
            //} else {
            //    return std::make_shared<ASTNode>(node->elseBlock);
            //}
        }
        return node;
    }

    std::shared_ptr<ASTNode> optimizeReturnValue(std::shared_ptr<ReturnValueNode> node) {
        node->value = optimize(node->value);
        if (auto num = dynamic_pointer_cast<NumberNode>(node->value)) {
            if (num->value == 0) {
                return std::make_shared<ReturnNode>(); // Remove redundant return 0
            }
        }
        return node;
    }
};

#endif // REWRITER_H
