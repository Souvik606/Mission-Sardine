#pragma once

#include <bits/stdc++.h>

#include "context.h"
#include "../ast_results/runtime_result.h"
#include "../ast_nodes/operation_nodes.h"
#include "../ast_nodes/variable_nodes.h"
#include "../ast_nodes/if_else_elif_nodes.h"
#include "../ast_nodes/for_nodes.h"
#include "../ast_nodes/while_nodes.h"
#include "../ast_nodes/function_nodes.h"
#include "../data_types/number_type.h"
#include "../data_types/function_type.h"
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
        visit_methods[typeid(VariableUseNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
            return Interpreter::visit_VariableUseNode(static_pointer_cast<VariableUseNode>(node), context);
        };
        visit_methods[typeid(VariableAssignNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
            return this->visit_VariableAssignNode(static_pointer_cast<VariableAssignNode>(node), context);
        };
        visit_methods[typeid(IfNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
            return this->visit_IfNode(static_pointer_cast<IfNode>(node), context);
        };
        visit_methods[typeid(ForNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
            return this->visit_ForNode(static_pointer_cast<ForNode>(node), context);
        };
        visit_methods[typeid(WhileNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
            return this->visit_WhileNode(static_pointer_cast<WhileNode>(node), context);
        };
        visit_methods[typeid(FunctionDefinitionNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
            return Interpreter::visit_FunctionDefinitionNode(static_pointer_cast<FunctionDefinitionNode>(node), context);
        };
        visit_methods[typeid(FunctionCallNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
            return this->visit_FunctionCallNode(static_pointer_cast<FunctionCallNode>(node), context);
        };
    }

    RunTimeResult visit(const shared_ptr<Node> &node, const shared_ptr<Context> &context) {
        if (!node) {
             return RunTimeResult().failure(RunTimeError({}, {}, "Internal error: Cannot visit null node", context));
        }
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

    static RunTimeResult visit_FunctionDefinitionNode(const shared_ptr<FunctionDefinitionNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        string func_name = node->var_name_tok.has_value() ? any_cast<string>(node->var_name_tok->value) : "";
        auto body_node = node->body_node;
        vector<string> arg_names;
        for(const auto& tok : node->arg_name_toks) {
            arg_names.push_back(any_cast<string>(tok.value));
        }

        const auto func_value = make_shared<Function>(func_name, body_node, arg_names);
        func_value->set_context(context).set_pos(node->pos_start, node->pos_end);

        if(node->var_name_tok.has_value()){
            context->symbol_table->set(func_name, func_value);
        }

        return res.success(func_value);
    }

    RunTimeResult visit_FunctionCallNode(const shared_ptr<FunctionCallNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        vector<shared_ptr<DataType>> args;

        const auto call_value = res.register_result(visit(node->call_node, context));
        if (res.error) return res;

        const auto copied_call_value = call_value->copy();
        copied_call_value->set_pos(node->pos_start, node->pos_end);

        for (const auto& arg_node : node->arg_nodes) {
            args.push_back(res.register_result(visit(arg_node, context)));
            if (res.error) return res;
        }

        if (const auto func_to_call = dynamic_pointer_cast<Function>(copied_call_value)) {
            const auto return_value = res.register_result(func_to_call->execute(args, *this));
            if (res.error) return res;
            return res.success(return_value);
        }

        return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "Value is not a function", context));
    }

    RunTimeResult visit_WhileNode(const shared_ptr<WhileNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        while (true) {
            auto condition_value = res.register_result(visit(node->condition_node, context));
            if (res.error) return res;
            bool is_truthy = false;
            if (const auto num = dynamic_pointer_cast<Number>(condition_value)) {
                is_truthy = std::visit([](auto val){ return val != 0; }, num->value);
            } else if (condition_value != nullptr) {
                is_truthy = true;
            }
            if (!is_truthy) break;
            res.register_result(visit(node->body_node, context));
            if (res.error) return res;
        }
        return res.success(nullptr);
    }

    RunTimeResult visit_ForNode(const shared_ptr<ForNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        auto start_value_res = res.register_result(visit(node->start_value_node, context));
        if (res.error) return res;
        auto start_num = dynamic_pointer_cast<Number>(start_value_res);
        if (!start_num) return res.failure(RunTimeError(node->start_value_node->pos_start.value_or(Position()), node->start_value_node->pos_end.value_or(Position()), "For loop start value must be a number", context));
        auto end_value_res = res.register_result(visit(node->end_value_node, context));
        if (res.error) return res;
        auto end_num = dynamic_pointer_cast<Number>(end_value_res);
        if (!end_num) return res.failure(RunTimeError(node->end_value_node->pos_start.value_or(Position()), node->end_value_node->pos_end.value_or(Position()), "For loop end value must be a number", context));
        shared_ptr<Number> step_num;
        if (node->step_value_node) {
            auto step_value = res.register_result(visit(node->step_value_node, context));
            if (res.error) return res;
            step_num = dynamic_pointer_cast<Number>(step_value);
            if (!step_num) return res.failure(RunTimeError(node->step_value_node->pos_start.value_or(Position()), node->step_value_node->pos_end.value_or(Position()), "For loop step value must be a number", context));
        } else {
            step_num = make_shared<Number>(1LL);
        }
        auto var_name = any_cast<string>(node->var_name_tok.value);
        if (bool all_integers = holds_alternative<long long>(start_num->value) && holds_alternative<long long>(end_num->value) && holds_alternative<long long>(step_num->value)) {
            long long start = get<long long>(start_num->value);
            long long end = get<long long>(end_num->value);
            long long step = get<long long>(step_num->value);
            for (long long i = start; (step >= 0) ? (i <= end) : (i >= end); i += step) {
                context->symbol_table->set(var_name, make_shared<Number>(i));
                res.register_result(visit(node->body_node, context));
                if (res.error) return res;
            }
        } else {
            double start = std::visit([](auto v){ return static_cast<double>(v); }, start_num->value);
            double end = std::visit([](auto v){ return static_cast<double>(v); }, end_num->value);
            double step = std::visit([](auto v){ return static_cast<double>(v); }, step_num->value);
            for (double i = start; (step >= 0) ? (i <= end) : (i >= end); i += step) {
                context->symbol_table->set(var_name, make_shared<Number>(i));
                res.register_result(visit(node->body_node, context));
                if (res.error) return res;
            }
        }
        return res.success(nullptr);
    }

    RunTimeResult visit_IfNode(const shared_ptr<IfNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        for (const auto& case_pair : node->cases) {
            auto condition_value = res.register_result(visit(case_pair.first, context));
            if (res.error) return res;
            bool is_truthy = false;
            if (const auto num = dynamic_pointer_cast<Number>(condition_value)) {
                is_truthy = std::visit([](auto val){ return val != 0; }, num->value);
            } else if (condition_value != nullptr) {
                is_truthy = true;
            }
            if (is_truthy) {
                const auto expr_value = res.register_result(visit(case_pair.second, context));
                if (res.error) return res;
                return res.success(expr_value);
            }
        }
        if (node->else_case) {
            const auto else_value = res.register_result(visit(node->else_case, context));
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
            return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "'" + var_name + "' is not defined", context));
        }
        const auto copied_value = value->copy();
        copied_value->set_pos(node->pos_start, node->pos_end).set_context(context);
        return res.success(copied_value);
    }

    RunTimeResult visit_VariableAssignNode(const shared_ptr<VariableAssignNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        const auto var_name = any_cast<string>(node->var_name_tok.value);
        const shared_ptr<DataType> value = res.register_result(visit(node->value_node, context));
        if (res.error) return res;
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
            return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "Invalid number value in token", context));
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
            return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "Left operand must be a number", context));
        }

        if (error) return res.failure(*error);

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
            return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "Unary operand must be a number", context));
        }

        if (error) return res.failure(*error);

        result->set_pos(node->pos_start, node->pos_end);
        return res.success(result);
    }
};