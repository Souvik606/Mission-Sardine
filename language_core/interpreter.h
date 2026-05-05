#pragma once

#include <bits/stdc++.h>
#include "context.h"
#include "../ast_results/runtime_result.h"
#include "../ast_nodes/operation_nodes.h"
#include "../ast_nodes/variable_nodes.h"
#include "../ast_nodes/if_else_elif_nodes.h"
#include "../ast_nodes/switch_nodes.h"
#include "../ast_nodes/for_nodes.h"
#include "../ast_nodes/while_nodes.h"
#include "../ast_nodes/function_nodes.h"
#include "../ast_nodes/string_nodes.h"
#include "../ast_nodes/list_nodes.h"
#include "../ast_nodes/dict_nodes.h"
#include "../ast_nodes/jump_nodes.h"
#include "../ast_nodes/try_catch_nodes.h"
#include "../data_types/list_type.h"
#include "../data_types/dict_type.h"
#include "../data_types/number_type.h"
#include "../data_types/string_type.h"
#include "../data_types/function_type.h"
#include "../data_types/builtins.h"
#include "error.h"
#include "lexer.h"
#include "constants.h"

using namespace std;

class Interpreter {
public:
    Interpreter() {
        visit_methods[typeid(NumberNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_NumberNode(static_pointer_cast<NumberNode>(node), context);
        };
        visit_methods[typeid(StringNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_StringNode(static_pointer_cast<StringNode>(node), context);
        };
        visit_methods[typeid(ListNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_ListNode(static_pointer_cast<ListNode>(node), context);
        };
        visit_methods[typeid(BinaryOperationNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_BinaryOperationNode(static_pointer_cast<BinaryOperationNode>(node), context);
        };
        visit_methods[typeid(TernaryOperationNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_TernaryOperationNode(static_pointer_cast<TernaryOperationNode>(node), context);
        };
        visit_methods[typeid(UnaryOperationNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_UnaryOperationNode(static_pointer_cast<UnaryOperationNode>(node), context);
        };
        visit_methods[typeid(VariableUseNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_VariableUseNode(static_pointer_cast<VariableUseNode>(node), context);
        };
        visit_methods[typeid(VariableAssignNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_VariableAssignNode(static_pointer_cast<VariableAssignNode>(node), context);
        };
        visit_methods[typeid(IfNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_IfNode(static_pointer_cast<IfNode>(node), context);
        };
        visit_methods[typeid(SwitchNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_SwitchNode(static_pointer_cast<SwitchNode>(node), context);
        };
        visit_methods[typeid(ForNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_ForNode(static_pointer_cast<ForNode>(node), context);
        };
        visit_methods[typeid(WhileNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_WhileNode(static_pointer_cast<WhileNode>(node), context);
        };
        visit_methods[typeid(FunctionDefinitionNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_FunctionDefinitionNode(static_pointer_cast<FunctionDefinitionNode>(node), context);
        };
        visit_methods[typeid(FunctionCallNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_FunctionCallNode(static_pointer_cast<FunctionCallNode>(node), context);
        };
        visit_methods[typeid(ReturnNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_ReturnNode(static_pointer_cast<ReturnNode>(node), context);
        };
        visit_methods[typeid(ContinueNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_ContinueNode(static_pointer_cast<ContinueNode>(node), context);
        };
        visit_methods[typeid(BreakNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_BreakNode(static_pointer_cast<BreakNode>(node), context);
        };
        visit_methods[typeid(DictNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_DictNode(static_pointer_cast<DictNode>(node), context);
        };
        visit_methods[typeid(TryNode)] = [this](const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
            return this->visit_TryNode(static_pointer_cast<TryNode>(node), context);
        };
    }

    RunTimeResult visit(const shared_ptr<Node>& node, const shared_ptr<Context>& context) {
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

    static RunTimeResult no_visit_method(const shared_ptr<Node>& node) {
        throw std::runtime_error("No visit method defined for node type: " + string(typeid(*node.get()).name()));
    }

    RunTimeResult visit_ListNode(const shared_ptr<ListNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        vector<shared_ptr<DataType>> elements;

        for (const auto& element_node : node->element_nodes) {
            elements.push_back(res.register_result(visit(element_node, context)));
            if (res.should_return()) return res;
        }

        auto list_value = make_shared<List>(elements);
        list_value->set_context(context).set_pos(node->pos_start, node->pos_end);
        return res.success(std::static_pointer_cast<DataType>(list_value));
    }

    RunTimeResult visit_StringNode(const shared_ptr<StringNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        shared_ptr<String> str;

        if (node->token.value.type() == typeid(string)) {
            str = make_shared<String>(any_cast<string>(node->token.value));
        } else {
            return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "Invalid value in string token", context));
        }

        str->set_context(context).set_pos(node->pos_start, node->pos_end);
        return res.success(std::static_pointer_cast<DataType>(str));
    }

    RunTimeResult visit_FunctionDefinitionNode(const shared_ptr<FunctionDefinitionNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        string func_name = node->var_name_tok.has_value() ? any_cast<string>(node->var_name_tok->value) : "";
        auto body_node = node->body_node;
        vector<string> arg_names;
        for (const auto& tok : node->arg_name_toks) {
            arg_names.push_back(any_cast<string>(tok.value));
        }

        auto func_value = make_shared<Function>(func_name, body_node, arg_names, node->return_null);
        func_value->set_context(context).set_pos(node->pos_start, node->pos_end);

        if (node->var_name_tok.has_value()) {
            context->symbol_table->set(func_name, func_value);
        }

        return res.success(std::static_pointer_cast<DataType>(func_value));
    }

    RunTimeResult visit_FunctionCallNode(const shared_ptr<FunctionCallNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        vector<shared_ptr<DataType>> args;

        auto call_value = res.register_result(visit(node->call_node, context));
        if (res.should_return()) return res;

        auto copied_call_value = call_value->copy();
        copied_call_value->set_pos(node->pos_start, node->pos_end);

        for (const auto& arg_node : node->arg_nodes) {
            args.push_back(res.register_result(visit(arg_node, context)));
            if (res.should_return()) return res;
        }

        shared_ptr<DataType> return_value;
        if (auto func_to_call = dynamic_pointer_cast<Function>(copied_call_value)) {
            return_value = res.register_result(func_to_call->execute(args, *this));
        } else if (auto builtin_to_call = dynamic_pointer_cast<BuiltInFunction>(copied_call_value)) {
            return_value = res.register_result(builtin_to_call->execute(args));
        } else {
            return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "Value is not a function", context));
        }

        if (res.should_return()) return res;

        auto final_return_val = return_value->copy();
        final_return_val->set_pos(node->pos_start, node->pos_end).set_context(context);

        return res.success(final_return_val);
    }

    RunTimeResult visit_WhileNode(const shared_ptr<WhileNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        vector<shared_ptr<DataType>> elements;

        while (true) {
            auto condition_value = res.register_result(visit(node->condition_node, context));
            if (res.should_return()) return res;

            auto [cond, error] = condition_value->is_true();
            if (error) return res.failure(*error);

            if (cond && !cond->is_truthy()) break;

            auto value = res.register_result(visit(node->body_node, context));
            if (res.should_return() && !res.loop_or_switch_break && !res.loop_continue) {
                return res;
            }

            if (res.loop_continue) continue;
            if (res.loop_or_switch_break) break;

            elements.push_back(value);
        }

        if (node->return_null) {
            auto null_val = make_shared<Number>(0LL);
            null_val->set_context(context).set_pos(node->pos_start, node->pos_end);
            return res.success(std::static_pointer_cast<DataType>(null_val));
        }

        auto list_value = make_shared<List>(elements);
        list_value->set_context(context).set_pos(node->pos_start, node->pos_end);
        return res.success(std::static_pointer_cast<DataType>(list_value));
    }

    RunTimeResult visit_ForNode(const shared_ptr<ForNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        vector<shared_ptr<DataType>> elements;

        auto start_value = res.register_result(visit(node->start_value_node, context));
        if (res.should_return()) return res;

        auto end_value = res.register_result(visit(node->end_value_node, context));
        if (res.should_return()) return res;

        shared_ptr<DataType> step_value;
        if (node->step_value_node) {
            step_value = res.register_result(visit(node->step_value_node, context));
            if (res.should_return()) return res;
        } else {
            step_value = std::static_pointer_cast<DataType>(make_shared<Number>(1LL));
        }

        auto start_num = dynamic_pointer_cast<Number>(start_value);
        auto end_num = dynamic_pointer_cast<Number>(end_value);
        auto step_num = dynamic_pointer_cast<Number>(step_value);

        if (!start_num || !end_num || !step_num) {
            return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "For loop parameters must be numbers", context));
        }

        auto var_name = any_cast<string>(node->var_name_tok.value);

        double i_val = holds_alternative<long long>(start_num->value) ? get<long long>(start_num->value) : get<double>(start_num->value);
        double end_val = holds_alternative<long long>(end_num->value) ? get<long long>(end_num->value) : get<double>(end_num->value);
        double step_val = holds_alternative<long long>(step_num->value) ? get<long long>(step_num->value) : get<double>(step_num->value);

        auto condition = [&]() { return step_val >= 0 ? (i_val <= end_val) : (i_val >= end_val); };

        while (condition()) {
            if (holds_alternative<long long>(start_num->value) && holds_alternative<long long>(step_num->value)) {
                context->symbol_table->set(var_name, std::static_pointer_cast<DataType>(make_shared<Number>((long long)i_val)));
            } else {
                context->symbol_table->set(var_name, std::static_pointer_cast<DataType>(make_shared<Number>(i_val)));
            }

            i_val += step_val;

            auto value = res.register_result(visit(node->body_node, context));
            if (res.should_return() && !res.loop_continue && !res.loop_or_switch_break) {
                return res;
            }

            if (res.loop_continue) continue;
            if (res.loop_or_switch_break) break;

            elements.push_back(value);
        }

        if (node->return_null) {
            auto null_val = make_shared<Number>(0LL);
            null_val->set_context(context).set_pos(node->pos_start, node->pos_end);
            return res.success(std::static_pointer_cast<DataType>(null_val));
        }

        auto list_value = make_shared<List>(elements);
        list_value->set_context(context).set_pos(node->pos_start, node->pos_end);
        return res.success(std::static_pointer_cast<DataType>(list_value));
    }

    RunTimeResult visit_SwitchNode(const shared_ptr<SwitchNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        vector<shared_ptr<DataType>> elements;

        auto selection_val = res.register_result(visit(node->switch_value, context));
        if (res.should_return()) return res;

        int match_index = 0, start_index = 0;
        int default_index = node->cases.size();
        bool match_found = false;

        for (const auto& c : node->cases) {
            if (c->value == nullptr) {
                default_index = match_index;
                match_index++;
                continue;
            }

            auto choice_val = res.register_result(visit(c->value, context));
            if (res.should_return()) return res;

            auto [comp_res, error] = selection_val->get_comparison_eq(choice_val);
            if (error) return res.failure(*error);

            if (comp_res && comp_res->is_truthy()) {
                match_found = true;
                break;
            }
            match_index++;
        }

        start_index = match_found ? match_index : default_index;

        for (size_t i = start_index; i < node->cases.size(); ++i) {
            const auto& c = node->cases[i];
            auto body_val = res.register_result(visit(c->body, context));

            if (res.should_return() && !res.loop_or_switch_break) return res;

            if (c->return_null) {
                elements.push_back(std::static_pointer_cast<DataType>(make_shared<Number>(0LL)));
            } else {
                elements.push_back(body_val);
            }

            if (res.loop_or_switch_break) break;
        }

        if (node->return_null) {
            auto null_val = make_shared<Number>(0LL);
            null_val->set_context(context).set_pos(node->pos_start, node->pos_end);
            return res.success(std::static_pointer_cast<DataType>(null_val));
        }

        auto list_val = make_shared<List>(elements);
        list_val->set_context(context).set_pos(node->pos_start, node->pos_end);
        return res.success(std::static_pointer_cast<DataType>(list_val));
    }

    RunTimeResult visit_IfNode(const shared_ptr<IfNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;

        for (const auto& case_tuple : node->cases) {
            auto condition_value = res.register_result(visit(get<0>(case_tuple), context));
            if (res.should_return()) return res;

            auto [cond, error] = condition_value->is_true();
            if (error) return res.failure(*error);

            if (cond && cond->is_truthy()) {
                auto expr_value = res.register_result(visit(get<1>(case_tuple), context));
                if (res.should_return()) return res;

                if (get<2>(case_tuple)) return res.success(std::static_pointer_cast<DataType>(make_shared<Number>(0LL)));
                return res.success(expr_value);
            }
        }

        if (node->else_case) {
            auto else_value = res.register_result(visit(node->else_case->first, context));
            if (res.should_return()) return res;

            if (node->else_case->second) return res.success(std::static_pointer_cast<DataType>(make_shared<Number>(0LL)));
            return res.success(else_value);
        }

        return res.success(std::static_pointer_cast<DataType>(make_shared<Number>(0LL)));
    }

    RunTimeResult visit_VariableUseNode(const shared_ptr<VariableUseNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        string var_name = any_cast<string>(node->var_name_tok.value);
        shared_ptr<DataType> value = context->symbol_table->get(var_name);

        if (!value) {
            return res.failure(RunTimeError(
                node->pos_start.value_or(Position()), node->pos_end.value_or(Position()),
                "'" + var_name + "' is not defined", context
            ));
        }

        vector<shared_ptr<DataType>> indexes;
        for (const auto& index : node->index_node) {
            auto index_val = res.register_result(visit(index, context));
            if (res.error) return res;
            indexes.push_back(index_val);
        }

        if (indexes.empty()) {
            auto copy_val = value->copy();
            copy_val->set_pos(node->pos_start, node->pos_end).set_context(context);
            return res.success(copy_val);
        } else {
            auto [indexed_val, error] = value->getByIndex(indexes);
            if (error) return res.failure(*error);

            auto copy_val = indexed_val->copy();
            copy_val->set_pos(node->pos_start, node->pos_end).set_context(context);
            return res.success(copy_val);
        }
    }

    RunTimeResult visit_VariableAssignNode(const shared_ptr<VariableAssignNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        shared_ptr<DataType> last_result = nullptr;

        for (size_t i = 0; i < node->var_name_toks.size(); ++i) {
            string var_name = any_cast<string>(node->var_name_toks[i].value);
            vector<shared_ptr<DataType>> indexes_vals;

            if (i < node->index_nodes.size() && !node->index_nodes[i].empty()) {
                for (const auto& index : node->index_nodes[i]) {
                    auto index_val = res.register_result(visit(index, context));
                    if (res.should_return()) return res;
                    indexes_vals.push_back(index_val);
                }
            }

            auto value = res.register_result(visit(node->value_nodes[i], context));
            if (res.should_return()) return res;

            if (!indexes_vals.empty()) {
                auto list_value = context->symbol_table->get(var_name);
                if (!list_value) {
                    return res.failure(RunTimeError(
                        node->var_name_toks[i].pos_start.value_or(Position()),
                        node->value_nodes[i]->pos_end.value_or(Position()),
                        "'" + var_name + "' is not defined", context
                    ));
                }

                auto [new_list, error] = list_value->assignIndex(indexes_vals, value);
                if (error) return res.failure(*error);

                context->symbol_table->set(var_name, new_list);
                last_result = new_list;
            } else {
                context->symbol_table->set(var_name, value);
                last_result = value;
            }
        }

        return res.success(last_result);
    }

    RunTimeResult visit_NumberNode(const shared_ptr<NumberNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        shared_ptr<Number> number;

        if (node->token.value.type() == typeid(long long)) {
            number = make_shared<Number>(any_cast<long long>(node->token.value));
        } else if (node->token.value.type() == typeid(double)) {
            number = make_shared<Number>(any_cast<double>(node->token.value));
        } else {
            return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "Invalid number format", context));
        }

        number->set_context(context).set_pos(node->pos_start, node->pos_end);
        return res.success(std::static_pointer_cast<DataType>(number));
    }

    RunTimeResult visit_ReturnNode(const shared_ptr<ReturnNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        shared_ptr<DataType> value = std::static_pointer_cast<DataType>(make_shared<Number>(0LL));
        if (node->node_to_return) {
            value = res.register_result(visit(node->node_to_return, context));
            if (res.should_return()) return res;
        }
        return res.success_return(value);
    }

    RunTimeResult visit_ContinueNode(const shared_ptr<ContinueNode>& node, const shared_ptr<Context>& context) {
        return RunTimeResult().success_continue();
    }

    RunTimeResult visit_BreakNode(const shared_ptr<BreakNode>& node, const shared_ptr<Context>& context) {
        return RunTimeResult().success_break();
    }

    RunTimeResult visit_BinaryOperationNode(const shared_ptr<BinaryOperationNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        const shared_ptr<DataType> left = res.register_result(visit(node->left_node, context));
        if (res.should_return()) return res;
        const shared_ptr<DataType> right = res.register_result(visit(node->right_node, context));
        if (res.should_return()) return res;

        shared_ptr<DataType> result = nullptr;
        optional<RunTimeError> error = nullopt;

        if (node->operator_token.type == T_PLUS) tie(result, error) = left->add(right);
        else if (node->operator_token.type == T_MINUS) tie(result, error) = left->subtract(right);
        else if (node->operator_token.type == T_MUL) tie(result, error) = left->multiply(right);
        else if (node->operator_token.type == T_DIVIDE) tie(result, error) = left->divide(right);
        else if (node->operator_token.type == T_MODULUS) tie(result, error) = left->modulus(right);
        else if (node->operator_token.type == T_FLOOR) tie(result, error) = left->floor_divide(right);
        else if (node->operator_token.type == T_EXP) tie(result, error) = left->exponent(right);
        else if (node->operator_token.type == T_EE) tie(result, error) = left->get_comparison_eq(right);
        else if (node->operator_token.type == T_NEQ) tie(result, error) = left->get_comparison_neq(right);
        else if (node->operator_token.type == T_LT) tie(result, error) = left->get_comparison_lt(right);
        else if (node->operator_token.type == T_GT) tie(result, error) = left->get_comparison_gt(right);
        else if (node->operator_token.type == T_LTE) tie(result, error) = left->get_comparison_lte(right);
        else if (node->operator_token.type == T_GTE) tie(result, error) = left->get_comparison_gte(right);
        else if (node->operator_token.type == T_KEYWORD) {
            if (any_cast<string>(node->operator_token.value) == "and") tie(result, error) = left->and_by(right);
            else if (any_cast<string>(node->operator_token.value) == "or") tie(result, error) = left->or_by(right);
        }

        if (error) return res.failure(*error);
        if (!result) return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "Unsupported operation", context));

        result->set_pos(node->pos_start, node->pos_end);
        return res.success(result);
    }

    RunTimeResult visit_TernaryOperationNode(const shared_ptr<TernaryOperationNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        auto comp_node = res.register_result(visit(node->comp_node, context));
        if (res.should_return()) return res;

        auto [cond, error] = comp_node->is_true();
        if (error) return res.failure(*error);

        shared_ptr<DataType> result;
        if (cond && cond->is_truthy()) {
            result = res.register_result(visit(node->true_node, context));
            if (res.should_return()) return res;
        } else {
            result = res.register_result(visit(node->false_node, context));
            if (res.should_return()) return res;
        }

        result->set_pos(node->pos_start, node->pos_end);
        return res.success(result);
    }

    RunTimeResult visit_UnaryOperationNode(const shared_ptr<UnaryOperationNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        auto number = res.register_result(visit(node->node, context));
        if (res.should_return()) return res;

        shared_ptr<DataType> result = nullptr;
        optional<RunTimeError> error = nullopt;

        if (node->operator_token.type == T_MINUS) {
            tie(result, error) = number->multiply(std::static_pointer_cast<DataType>(make_shared<Number>(-1LL)));
        } else if (node->operator_token.type == T_KEYWORD && any_cast<string>(node->operator_token.value) == "not") {
            tie(result, error) = number->not_by();
        }

        if (error) return res.failure(*error);
        if (!result) return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "Unsupported unary operation", context));

        result->set_pos(node->pos_start, node->pos_end);
        return res.success(result);
    }

    RunTimeResult visit_DictNode(const shared_ptr<DictNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        vector<pair<shared_ptr<DataType>, shared_ptr<DataType>>> elements;

        for (const auto& pair : node->keyval_nodes) {
            auto key = res.register_result(visit(pair.first, context));
            if (res.should_return()) return res;

            if (!dynamic_cast<Number*>(key.get()) && !dynamic_cast<String*>(key.get())) {
                return res.failure(IllegalOperationError(pair.first->pos_start.value_or(Position{}), pair.first->pos_end.value_or(Position{}), "Dictionary keys must be numbers or strings", context));
            }

            auto value = res.register_result(visit(pair.second, context));
            if (res.should_return()) return res;

            elements.push_back({key, value});
        }

        auto dict_val = make_shared<Dict>(elements);
        dict_val->set_context(context);
        dict_val->set_pos(node->pos_start.value_or(Position{}), node->pos_end.value_or(Position{}));
        
        return res.success(dict_val);
    }

    RunTimeResult visit_TryNode(const shared_ptr<TryNode>& node, const shared_ptr<Context>& context) {
        RunTimeResult res;
        auto try_result = res.register_result(visit(node->body_node, context));

        if (!res.error) {
            if (node->clean_node) {
                res.register_result(visit(node->clean_node->body_node, context));
                if (res.should_return()) return res;
            }
            return res.success(try_result);
        }

        auto error = *res.error;
        res.error = nullopt;
        bool handled = false;

        for (const auto& trap_node : node->trap_nodes) {
            bool matches = false;
            if (!trap_node->error_type) {
                matches = true;
            } else {
                string caught_err = any_cast<string>(trap_node->error_type->value);
                if (caught_err == "RunTimeError" || caught_err == error.error_name) {
                    matches = true;
                } else if (find(ERROR_TYPES.begin(), ERROR_TYPES.end(), caught_err) == ERROR_TYPES.end()) {
                    return res.failure(InvalidErrorTypeError(trap_node->pos_start.value_or(Position{}), trap_node->pos_end.value_or(Position{}), "'" + caught_err + "' is not a valid error type", context));
                }
            }

            if (matches) {
                auto trap_context = make_shared<Context>("<trap block>", context, trap_node->pos_start.value_or(Position{}));
                trap_context->symbol_table = make_shared<SymbolTable>(context->symbol_table);

                if (trap_node->error_name) {
                    auto err_str = make_shared<String>(error.to_string());
                    err_str->set_pos(trap_node->pos_start.value_or(Position{}), trap_node->pos_end.value_or(Position{}));
                    err_str->set_context(trap_context);
                    trap_context->symbol_table->set(any_cast<string>(trap_node->error_name->value), err_str);
                }

                res.register_result(visit(trap_node->body_node, trap_context));
                if (res.error) return res;
                handled = true;
                break;
            }
        }

        if (node->clean_node) {
            res.register_result(visit(node->clean_node->body_node, context));
            if (res.should_return()) return res;
        }

        if (handled) return res.success(nullptr);

        return res.failure(error);
    }
};