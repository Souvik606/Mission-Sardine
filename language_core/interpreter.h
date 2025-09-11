#pragma once

#include <bits/stdc++.h>
#include "context.h"
#include "../ast_results/runtime_result.h"
#include "../ast_nodes/operation_nodes.h"
#include "../data_types/number_type.h"
#include "error.h"
#include "lexer.h"
#include "constants.h"

using namespace std;

class Interpreter {
public:
    Interpreter() {
        visit_methods[typeid(NumberNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
            return Interpreter::visit_NumberNode(static_pointer_cast<NumberNode>(node), context);
        };
        visit_methods[typeid(BinaryOperationNode)] = [this](const shared_ptr<Node> &node,
                                                            const shared_ptr<Context> &context) {
            return this->visit_BinaryOperationNode(static_pointer_cast<BinaryOperationNode>(node), context);
        };
        visit_methods[typeid(UnaryOperationNode)] = [this](const shared_ptr<Node> &node,
                                                           const shared_ptr<Context> &context) {
            return this->visit_UnaryOperationNode(static_pointer_cast<UnaryOperationNode>(node), context);
        };
    }

    RunTimeResult visit(const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
        const std::type_index type_idx = typeid(*node);

        if (const auto it = visit_methods.find(type_idx); it != visit_methods.end()) {
            return it->second(node, context);
        }
        return no_visit_method(node);
    }

private:
    using VisitFunction = std::function<RunTimeResult(shared_ptr<Node>, shared_ptr<Context>)>;
    std::unordered_map<std::type_index, VisitFunction> visit_methods;

    static RunTimeResult no_visit_method(const shared_ptr<Node> &node) {
        throw std::runtime_error("No visit method defined for node type: " + string(typeid(*node).name()));
    }

    static RunTimeResult visit_NumberNode(const shared_ptr<NumberNode> &node, const shared_ptr<Context> &context) {
        RunTimeResult res;
        auto token_value = node->token.value;
        shared_ptr<Number> number;

        if (token_value.type() == typeid(long long)) {
            number = make_shared<Number>(any_cast<long long>(token_value));
        } else if (token_value.type() == typeid(double)) {
            number = make_shared<Number>(any_cast<double>(token_value));
        } else {
            return res.failure(RunTimeError(
                node->pos_start.value_or(Position()),
                node->pos_end.value_or(Position()),
                "Invalid number value in token", context
            ));
        }

        number->set_context(context).set_pos(node->pos_start, node->pos_end);
        return res.success(number);
    }

    RunTimeResult visit_BinaryOperationNode(const shared_ptr<BinaryOperationNode> &node,
                                            const shared_ptr<Context> &context) {
        RunTimeResult res;
        const shared_ptr<DataType> left = res.register_result(visit(node->left_node, context));
        if (res.error) return res;

        const shared_ptr<DataType> right = res.register_result(visit(node->right_node, context));
        if (res.error) return res;

        shared_ptr<Number> result = nullptr;
        optional<RunTimeError> error = nullopt;

        if (const auto left_num = dynamic_pointer_cast<Number>(left)) {
            if (node->operator_token.type == T_PLUS) {
                tie(result, error) = left_num->add(right);
            } else if (node->operator_token.type == T_MINUS) {
                tie(result, error) = left_num->subtract(right);
            } else if (node->operator_token.type == T_MUL) {
                tie(result, error) = left_num->multiply(right);
            } else if (node->operator_token.type == T_DIVIDE) {
                tie(result, error) = left_num->divide(right);
            }
        } else {
            return res.failure(RunTimeError(
                node->pos_start.value_or(Position()),
                node->pos_end.value_or(Position()),
                "Left operand must be a number", context
            ));
        }

        if (error) {
            return res.failure(*error);
        } else {
            result->set_pos(node->pos_start, node->pos_end);
            return res.success(result);
        }
    }

    RunTimeResult visit_UnaryOperationNode(const shared_ptr<UnaryOperationNode> &node,
                                           const shared_ptr<Context> &context) {
        RunTimeResult res;
        const shared_ptr<DataType> number_val = res.register_result(visit(node->node, context));
        if (res.error) return res;

        optional<RunTimeError> error = nullopt;
        shared_ptr<Number> result = nullptr;

        if (const auto number = dynamic_pointer_cast<Number>(number_val)) {
            if (node->operator_token.type == T_MINUS) {
                tie(result, error) = number->multiply(make_shared<Number>(-1LL));
            }
        } else {
            return res.failure(RunTimeError(
                node->pos_start.value_or(Position()),
                node->pos_end.value_or(Position()),
                "Unary operand must be a number", context
            ));
        }

        if (error) {
            return res.failure(*error);
        } else {
            result->set_pos(node->pos_start, node->pos_end);
            return res.success(result);
        }
    }
};
