#pragma once

#include <bits/stdc++.h>

#include "context.h"
#include "../ast_results/runtime_result.h"
#include "../ast_nodes/operation_nodes.h"
#include "../ast_nodes/variable_nodes.h"
#include "../ast_nodes/if_else_elif_nodes.h"
#include "../data_types/number_type.h"
#include "error.h"
#include "lexer.h"
#include "constants.h"

using namespace std;

class Interpreter {
public:
    Interpreter() {
        visit_methods[typeid(NumberNode)] = [](const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
            return visit_NumberNode(static_pointer_cast<NumberNode>(node), context);
        };
        visit_methods[typeid(BinaryOperationNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
            return this->visit_BinaryOperationNode(static_pointer_cast<BinaryOperationNode>(node), context);
        };
        visit_methods[typeid(UnaryOperationNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
            return this->visit_UnaryOperationNode(static_pointer_cast<UnaryOperationNode>(node), context);
        };
        visit_methods[typeid(VariableUseNode)] = [](const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
            return visit_VariableUseNode(static_pointer_cast<VariableUseNode>(node), context);
        };
        visit_methods[typeid(VariableAssignNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
            return this->visit_VariableAssignNode(static_pointer_cast<VariableAssignNode>(node), context);
        };
        visit_methods[typeid(IfNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
            return this->visit_IfNode(static_pointer_cast<IfNode>(node), context);
        };
    }

    RunTimeResult visit(const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
        const std::type_index type_idx = typeid(*node.get());
        if (const auto it = visit_methods.find(type_idx); it != visit_methods.end()) {
            return it->second(node, context);
        }
        return no_visit_method(node);
    }

private:
    using VisitFunction = std::function<RunTimeResult(shared_ptr<Node>, shared_ptr<Context>)>;
    std::unordered_map<std::type_index, VisitFunction> visit_methods;

    static RunTimeResult no_visit_method(const shared_ptr<Node> &node) {
        throw std::runtime_error("No visit method defined for node type: " + string(typeid(*node.get()).name()));
    }

    RunTimeResult visit_IfNode(const shared_ptr<IfNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        for (const auto& case_pair : node->cases) {
            auto condition_node = case_pair.first;
            auto expression_node = case_pair.second;

            auto condition_value = res.register_result(visit(condition_node, context));
            if (res.error) return res;

            bool is_truthy = false;
            if (const auto num = dynamic_pointer_cast<Number>(condition_value)) {
                is_truthy = std::visit([](auto val){ return val != 0; }, num->value);
            } else {
                 return res.failure(RunTimeError(
                    condition_node->pos_start.value_or(Position()),
                    condition_node->pos_end.value_or(Position()),
                    "Condition must evaluate to a number", context
                ));
            }

            if (is_truthy) {
                auto expr_value = res.register_result(visit(expression_node, context));
                if (res.error) return res;
                return res.success(expr_value);
            }
        }

        if (node->else_case) {
            auto else_value = res.register_result(visit(node->else_case, context));
            if (res.error) return res;
            return res.success(else_value);
        }

        return res.success(nullptr);
    }

    static RunTimeResult visit_VariableUseNode(const shared_ptr<VariableUseNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        const auto var_name = any_cast<string>(node->var_name_tok.value);
        const shared_ptr<DataType> value = context->symbol_table->get(var_name);
        if (!value) {
            return res.failure(RunTimeError(
                node->pos_start.value_or(Position()),
                node->pos_end.value_or(Position()),
                "'" + var_name + "' is not defined",
                context
            ));
        }
        return res.success(value);
    }

    RunTimeResult visit_VariableAssignNode(const shared_ptr<VariableAssignNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        const auto var_name = any_cast<string>(node->var_name_tok.value);
        const shared_ptr<DataType> value = res.register_result(visit(node->value_node, context));
        if (res.error) {
            return res;
        }
        context->symbol_table->set(var_name, value);
        return res.success(value);
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
            } else if (node->operator_token.type == T_EE) {
                tie(result, error) = left_num->get_comparison_eq(right);
            } else if (node->operator_token.type == T_NEQ) {
                tie(result, error) = left_num->get_comparison_neq(right);
            } else if (node->operator_token.type == T_LT) {
                tie(result, error) = left_num->get_comparison_lt(right);
            } else if (node->operator_token.type == T_GT) {
                tie(result, error) = left_num->get_comparison_gt(right);
            } else if (node->operator_token.type == T_LTE) {
                tie(result, error) = left_num->get_comparison_lte(right);
            } else if (node->operator_token.type == T_GTE) {
                tie(result, error) = left_num->get_comparison_gte(right);
            } else if (node->operator_token.type == T_KEYWORD) {
                if (const auto keyword = any_cast<string>(node->operator_token.value); keyword == "and") {
                    tie(result, error) = left_num->and_by(right);
                } else if (keyword == "or") {
                    tie(result, error) = left_num->or_by(right);
                }
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
        }

        result->set_pos(node->pos_start, node->pos_end);
        return res.success(result);
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
            } else if (node->operator_token.type == T_KEYWORD) {
                if (const auto keyword = any_cast<string>(node->operator_token.value); keyword == "not") {
                    tie(result, error) = number->not_by();
                }
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
        }

        result->set_pos(node->pos_start, node->pos_end);
        return res.success(result);
    }
};