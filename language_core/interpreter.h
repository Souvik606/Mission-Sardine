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
#include "../ast_nodes/class_nodes.h"
#include "../data_types/list_type.h"
#include "../data_types/dict_type.h"
#include "../data_types/number_type.h"
#include "../data_types/string_type.h"
#include "../data_types/function_type.h"
#include "../data_types/builtins.h"
#include "../data_types/model_type.h"
#include "error.h"
#include "lexer.h"
#include "constants.h"

using namespace std;

class Interpreter
{
public:
    Interpreter()
    {
        visit_methods[typeid(NumberNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_NumberNode(static_pointer_cast<NumberNode>(node), context);
        };
        visit_methods[typeid(StringNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_StringNode(static_pointer_cast<StringNode>(node), context);
        };
        visit_methods[typeid(ListNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_ListNode(static_pointer_cast<ListNode>(node), context);
        };
        visit_methods[typeid(BinaryOperationNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_BinaryOperationNode(static_pointer_cast<BinaryOperationNode>(node), context);
        };
        visit_methods[typeid(TernaryOperationNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_TernaryOperationNode(static_pointer_cast<TernaryOperationNode>(node), context);
        };
        visit_methods[typeid(UnaryOperationNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_UnaryOperationNode(static_pointer_cast<UnaryOperationNode>(node), context);
        };
        visit_methods[typeid(VariableUseNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_VariableUseNode(static_pointer_cast<VariableUseNode>(node), context);
        };
        visit_methods[typeid(VariableAssignNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_VariableAssignNode(static_pointer_cast<VariableAssignNode>(node), context);
        };
        visit_methods[typeid(IfNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_IfNode(static_pointer_cast<IfNode>(node), context);
        };
        visit_methods[typeid(SwitchNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_SwitchNode(static_pointer_cast<SwitchNode>(node), context);
        };
        visit_methods[typeid(ForNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_ForNode(static_pointer_cast<ForNode>(node), context);
        };
        visit_methods[typeid(WhileNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_WhileNode(static_pointer_cast<WhileNode>(node), context);
        };
        visit_methods[typeid(FunctionDefinitionNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_FunctionDefinitionNode(static_pointer_cast<FunctionDefinitionNode>(node), context);
        };
        visit_methods[typeid(FunctionCallNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_FunctionCallNode(static_pointer_cast<FunctionCallNode>(node), context);
        };
        visit_methods[typeid(ReturnNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_ReturnNode(static_pointer_cast<ReturnNode>(node), context);
        };
        visit_methods[typeid(ContinueNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_ContinueNode(static_pointer_cast<ContinueNode>(node), context);
        };
        visit_methods[typeid(BreakNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_BreakNode(static_pointer_cast<BreakNode>(node), context);
        };
        visit_methods[typeid(DictNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_DictNode(static_pointer_cast<DictNode>(node), context);
        };
        visit_methods[typeid(TryNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_TryNode(static_pointer_cast<TryNode>(node), context);
        };
        visit_methods[typeid(ModelNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_ModelNode(static_pointer_cast<ModelNode>(node), context);
        };
        visit_methods[typeid(AttrAccessNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_AttrAccessNode(static_pointer_cast<AttrAccessNode>(node), context);
        };
        visit_methods[typeid(AttrAssignNode)] = [this](const shared_ptr<Node> &node, const shared_ptr<Context> &context)
        {
            return this->visit_AttrAssignNode(static_pointer_cast<AttrAssignNode>(node), context);
        };
    }

    RunTimeResult visit(const shared_ptr<Node> &node, const shared_ptr<Context> &context)
    {
        if (!node)
        {
            return RunTimeResult().failure(RunTimeError({}, {}, "Internal error: Cannot visit null node", context));
        }
        const std::type_index type_idx = typeid(*node.get());
        if (const auto it = visit_methods.find(type_idx); it != visit_methods.end())
        {
            return it->second(node, context);
        }
        return no_visit_method(node);
    }

private:
    using VisitFunction = std::function<RunTimeResult(shared_ptr<Node>, shared_ptr<Context>)>;
    std::unordered_map<std::type_index, VisitFunction> visit_methods;

    static RunTimeResult no_visit_method(const shared_ptr<Node> &node)
    {
        throw std::runtime_error("No visit method defined for node type: " + string(typeid(*node.get()).name()));
    }

public:
    shared_ptr<SymbolTable> get_instance_symbol_table(const shared_ptr<DataType> &val) const
    {
        if (auto inst = dynamic_pointer_cast<ModelInstance>(val))
        {
            return inst->symbol_table;
        }
        return nullptr;
    }

private:
    RunTimeResult visit_ListNode(const shared_ptr<ListNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;
        vector<shared_ptr<DataType>> elements;

        for (const auto &element_node : node->element_nodes)
        {
            elements.push_back(res.register_result(visit(element_node, context)));
            if (res.should_return())
                return res;
        }

        auto list_value = make_shared<List>(elements);
        list_value->set_context(context).set_pos(node->pos_start, node->pos_end);
        return res.success(std::static_pointer_cast<DataType>(list_value));
    }

    RunTimeResult visit_StringNode(const shared_ptr<StringNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;
        shared_ptr<String> str;

        if (node->token.value.type() == typeid(string))
        {
            str = make_shared<String>(any_cast<string>(node->token.value));
        }
        else
        {
            return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "Invalid value in string token", context));
        }

        str->set_context(context).set_pos(node->pos_start, node->pos_end);
        return res.success(std::static_pointer_cast<DataType>(str));
    }

    RunTimeResult visit_FunctionDefinitionNode(const shared_ptr<FunctionDefinitionNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;
        string func_name = node->var_name_tok.has_value() ? any_cast<string>(node->var_name_tok->value) : "";
        auto body_node = node->body_node;
        vector<pair<string, shared_ptr<Node>>> arg_nodes;
        for (const auto &p : node->arg_nodes)
        {
            arg_nodes.push_back({any_cast<string>(p.first.value), p.second});
        }

        auto func_value = make_shared<Function>(func_name, body_node, arg_nodes, node->return_null);
        func_value->set_context(context).set_pos(node->pos_start, node->pos_end);

        if (node->var_name_tok.has_value())
        {
            context->symbol_table->set(func_name, func_value);
        }

        return res.success(std::static_pointer_cast<DataType>(func_value));
    }

    RunTimeResult visit_FunctionCallNode(const shared_ptr<FunctionCallNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;
        vector<shared_ptr<DataType>> pos_args;
        map<string, shared_ptr<DataType>> kw_args;

        auto call_value = res.register_result(visit(node->call_node, context));
        if (res.should_return())
            return res;

        call_value->set_pos(node->pos_start, node->pos_end);

        for (const auto &arg_node : node->positional_arg_nodes)
        {
            pos_args.push_back(res.register_result(visit(arg_node, context)));
            if (res.should_return())
                return res;
        }

        for (const auto &p : node->keyword_arg_nodes)
        {
            const string &name = any_cast<string>(p.first.value);
            auto val = res.register_result(visit(p.second, context));
            if (res.should_return())
                return res;
            kw_args[name] = val;
        }

        shared_ptr<DataType> return_value;
        if (auto func_to_call = dynamic_pointer_cast<Function>(call_value))
        {
            return_value = res.register_result(func_to_call->execute(pos_args, kw_args, *this));
        }
        else if (auto builtin_to_call = dynamic_pointer_cast<BuiltInFunction>(call_value))
        {
            return_value = res.register_result(builtin_to_call->execute(pos_args, kw_args));
        }
        else if (auto model_to_call = dynamic_pointer_cast<ModelType>(call_value))
        {
            return_value = res.register_result(model_to_call->execute(pos_args, kw_args, *this));
        }
        else
        {
            return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "Value is not callable", context));
        }

        if (res.should_return())
            return res;

        if (return_value)
        {
            return_value->set_pos(node->pos_start, node->pos_end).set_context(context);
        }
        return res.success(return_value);
    }

    RunTimeResult visit_WhileNode(const shared_ptr<WhileNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;

        while (true)
        {
            auto condition_value = res.register_result(visit(node->condition_node, context));
            if (res.should_return())
                return res;

            auto [cond, error] = condition_value->is_true();
            if (error)
                return res.failure(*error);

            if (cond && !cond->is_truthy())
                break;

            res.register_result(visit(node->body_node, context));
            if (res.should_return() && !res.loop_or_switch_break && !res.loop_continue)
            {
                return res;
            }

            if (res.loop_continue)
            {
                res.loop_continue = false;
                continue;
            }
            if (res.loop_or_switch_break)
            {
                res.loop_or_switch_break = false;
                break;
            }
        }

        return res.success(std::static_pointer_cast<DataType>(Number::make(0LL)));
    }

    RunTimeResult visit_ForNode(const shared_ptr<ForNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;

        auto start_value = res.register_result(visit(node->start_value_node, context));
        if (res.should_return())
            return res;

        auto end_value = res.register_result(visit(node->end_value_node, context));
        if (res.should_return())
            return res;

        shared_ptr<DataType> step_value;
        if (node->step_value_node)
        {
            step_value = res.register_result(visit(node->step_value_node, context));
            if (res.should_return())
                return res;
        }
        else
        {
            step_value = std::static_pointer_cast<DataType>(Number::make(1LL));
        }

        auto start_num = dynamic_pointer_cast<Number>(start_value);
        auto end_num = dynamic_pointer_cast<Number>(end_value);
        auto step_num = dynamic_pointer_cast<Number>(step_value);

        if (!start_num || !end_num || !step_num)
        {
            return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "For loop parameters must be numbers", context));
        }

        const string &var_name = any_cast<string>(node->var_name_tok.value);

        // ── Fast integer-only path ────────────────────────────────────────────
        if (holds_alternative<long long>(start_num->value) &&
            holds_alternative<long long>(end_num->value) &&
            holds_alternative<long long>(step_num->value))
        {

            long long i = get<long long>(start_num->value);
            long long end = get<long long>(end_num->value);
            long long step = get<long long>(step_num->value);

            auto cond = [&]()
            { return step >= 0 ? (i <= end) : (i >= end); };
            while (cond())
            {
                context->symbol_table->set(var_name, Number::make(i));
                i += step;

                res.register_result(visit(node->body_node, context));
                if (res.should_return() && !res.loop_continue && !res.loop_or_switch_break)
                    return res;
                if (res.loop_continue)
                {
                    res.loop_continue = false;
                    continue;
                }
                if (res.loop_or_switch_break)
                {
                    res.loop_or_switch_break = false;
                    break;
                }
            }
            return res.success(std::static_pointer_cast<DataType>(Number::make(0LL)));
        }

        // ── Floating-point path ───────────────────────────────────────────────
        double i_val = holds_alternative<long long>(start_num->value) ? (double)get<long long>(start_num->value) : get<double>(start_num->value);
        double end_val = holds_alternative<long long>(end_num->value) ? (double)get<long long>(end_num->value) : get<double>(end_num->value);
        double step_val = holds_alternative<long long>(step_num->value) ? (double)get<long long>(step_num->value) : get<double>(step_num->value);

        auto cond_f = [&]()
        { return step_val >= 0 ? (i_val <= end_val) : (i_val >= end_val); };
        while (cond_f())
        {
            context->symbol_table->set(var_name, std::static_pointer_cast<DataType>(make_shared<Number>(i_val)));
            i_val += step_val;

            res.register_result(visit(node->body_node, context));
            if (res.should_return() && !res.loop_continue && !res.loop_or_switch_break)
                return res;
            if (res.loop_continue)
            {
                res.loop_continue = false;
                continue;
            }
            if (res.loop_or_switch_break)
            {
                res.loop_or_switch_break = false;
                break;
            }
        }
        return res.success(std::static_pointer_cast<DataType>(Number::make(0LL)));
    }

    RunTimeResult visit_SwitchNode(const shared_ptr<SwitchNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;
        vector<shared_ptr<DataType>> elements;

        auto selection_val = res.register_result(visit(node->switch_value, context));
        if (res.should_return())
            return res;

        int match_index = 0, start_index = 0;
        int default_index = node->cases.size();
        bool match_found = false;

        for (const auto &c : node->cases)
        {
            if (c->value == nullptr)
            {
                default_index = match_index;
                match_index++;
                continue;
            }

            auto choice_val = res.register_result(visit(c->value, context));
            if (res.should_return())
                return res;

            auto [comp_res, error] = selection_val->get_comparison_eq(choice_val);
            if (error)
                return res.failure(*error);

            if (comp_res && comp_res->is_truthy())
            {
                match_found = true;
                break;
            }
            match_index++;
        }

        start_index = match_found ? match_index : default_index;

        for (size_t i = start_index; i < node->cases.size(); ++i)
        {
            const auto &c = node->cases[i];
            auto body_val = res.register_result(visit(c->body, context));

            if (res.should_return() && !res.loop_or_switch_break)
                return res;

            if (c->return_null)
            {
                elements.push_back(std::static_pointer_cast<DataType>(make_shared<Number>(0LL)));
            }
            else
            {
                elements.push_back(body_val);
            }

            if (res.loop_or_switch_break)
                break;
        }

        if (node->return_null)
        {
            auto null_val = make_shared<Number>(0LL);
            null_val->set_context(context).set_pos(node->pos_start, node->pos_end);
            return res.success(std::static_pointer_cast<DataType>(null_val));
        }

        auto list_val = make_shared<List>(elements);
        list_val->set_context(context).set_pos(node->pos_start, node->pos_end);
        return res.success(std::static_pointer_cast<DataType>(list_val));
    }

    RunTimeResult visit_IfNode(const shared_ptr<IfNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;

        for (const auto &case_tuple : node->cases)
        {
            auto condition_value = res.register_result(visit(get<0>(case_tuple), context));
            if (res.should_return())
                return res;

            auto [cond, error] = condition_value->is_true();
            if (error)
                return res.failure(*error);

            if (cond && cond->is_truthy())
            {
                auto expr_value = res.register_result(visit(get<1>(case_tuple), context));
                if (res.should_return())
                    return res;

                if (get<2>(case_tuple))
                    return res.success(std::static_pointer_cast<DataType>(make_shared<Number>(0LL)));
                return res.success(expr_value);
            }
        }

        if (node->else_case)
        {
            auto else_value = res.register_result(visit(node->else_case->first, context));
            if (res.should_return())
                return res;

            if (node->else_case->second)
                return res.success(std::static_pointer_cast<DataType>(make_shared<Number>(0LL)));
            return res.success(else_value);
        }

        return res.success(std::static_pointer_cast<DataType>(make_shared<Number>(0LL)));
    }

    RunTimeResult visit_VariableUseNode(const shared_ptr<VariableUseNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;
        const string &var_name = any_cast<string>(node->var_name_tok.value);
        shared_ptr<DataType> value = context->symbol_table->get(var_name);

        if (!value)
        {
            auto this_val = context->symbol_table->get("this");
            if (this_val)
            {
                if (auto inst = dynamic_pointer_cast<ModelInstance>(this_val))
                {
                    value = inst->symbol_table->get(var_name);
                    if (!value)
                    {
                        auto [method_val, err] = inst->get_attr(var_name, *this, context);
                        if (!err && method_val)
                            value = method_val;
                    }
                }
            }
        }

        if (!value)
        {
            return res.failure(NameError(
                node->pos_start.value_or(Position()), node->pos_end.value_or(Position()),
                "'" + var_name + "' is not defined", context));
        }

        if (node->index_node.empty())
        {
            return res.success(value);
        }

        vector<shared_ptr<DataType>> indexes;
        indexes.reserve(node->index_node.size());
        for (const auto &index : node->index_node)
        {
            auto index_val = res.register_result(visit(index, context));
            if (res.error)
                return res;
            indexes.push_back(std::move(index_val));
        }

        auto [indexed_val, error] = value->getByIndex(indexes);
        if (error)
            return res.failure(*error);
        return res.success(indexed_val);
    }

    RunTimeResult visit_VariableAssignNode(const shared_ptr<VariableAssignNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;
        shared_ptr<DataType> last_result = nullptr;

        for (size_t i = 0; i < node->var_name_toks.size(); ++i)
        {
            string var_name = any_cast<string>(node->var_name_toks[i].value);
            vector<shared_ptr<DataType>> indexes_vals;

            if (i < node->index_nodes.size() && !node->index_nodes[i].empty())
            {
                for (const auto &index : node->index_nodes[i])
                {
                    auto index_val = res.register_result(visit(index, context));
                    if (res.should_return())
                        return res;
                    indexes_vals.push_back(index_val);
                }
            }

            auto value = res.register_result(visit(node->value_nodes[i], context));
            if (res.should_return())
                return res;

            if (!indexes_vals.empty())
            {
                auto list_value = context->symbol_table->get(var_name);
                if (!list_value)
                {
                    return res.failure(NameError(
                        node->var_name_toks[i].pos_start.value_or(Position()),
                        node->value_nodes[i]->pos_end.value_or(Position()),
                        "'" + var_name + "' is not defined", context));
                }

                if (indexes_vals.size() == 1)
                {
                    if (const auto lst = dynamic_pointer_cast<List>(list_value))
                    {
                        if (const auto num_idx = dynamic_cast<const Number *>(indexes_vals[0].get()))
                        {
                            if (holds_alternative<long long>(num_idx->value))
                            {
                                long long idx = get<long long>(num_idx->value);
                                if (idx < 0 || idx >= static_cast<long long>(lst->elements.size()))
                                {
                                    return res.failure(IndexOutOfBoundsError(
                                        node->var_name_toks[i].pos_start.value_or(Position()),
                                        node->value_nodes[i]->pos_end.value_or(Position()),
                                        "Index out of bounds", context));
                                }
                                lst->elements[idx] = value;
                                last_result = list_value;
                                continue;
                            }
                        }
                    }
                }

                auto [new_list, error] = list_value->assignIndex(indexes_vals, value);
                if (error)
                    return res.failure(*error);

                context->symbol_table->set(var_name, new_list);
                last_result = new_list;
            }
            else
            {
                auto this_val = context->symbol_table->get("this");
                bool written_to_instance = false;

                if (this_val)
                {
                    if (auto inst = dynamic_pointer_cast<ModelInstance>(this_val))
                    {
                        bool is_attr = inst->symbol_table->get(var_name) != nullptr;
                        if (!is_attr)
                        {
                            is_attr = inst->model->find_attribute(var_name) != nullptr;
                        }
                        if (is_attr)
                        {
                            inst->symbol_table->set(var_name, value);
                            written_to_instance = true;
                        }
                    }
                }

                if (!written_to_instance)
                {
                    context->symbol_table->set(var_name, value);
                }
                last_result = value;
            }
        }

        return res.success(last_result);
    }

    RunTimeResult visit_NumberNode(const shared_ptr<NumberNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;
        shared_ptr<Number> number;

        if (node->token.value.type() == typeid(long long))
        {
            number = make_shared<Number>(any_cast<long long>(node->token.value));
        }
        else if (node->token.value.type() == typeid(double))
        {
            number = make_shared<Number>(any_cast<double>(node->token.value));
        }
        else
        {
            return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "Invalid number format", context));
        }

        number->set_context(context).set_pos(node->pos_start, node->pos_end);
        return res.success(std::static_pointer_cast<DataType>(number));
    }

    RunTimeResult visit_ReturnNode(const shared_ptr<ReturnNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;
        shared_ptr<DataType> value = std::static_pointer_cast<DataType>(make_shared<Number>(0LL));
        if (node->node_to_return)
        {
            value = res.register_result(visit(node->node_to_return, context));
            if (res.should_return())
                return res;
        }
        return res.success_return(value);
    }

    RunTimeResult visit_ContinueNode(const shared_ptr<ContinueNode> &node, const shared_ptr<Context> &context)
    {
        return RunTimeResult().success_continue();
    }

    RunTimeResult visit_BreakNode(const shared_ptr<BreakNode> &node, const shared_ptr<Context> &context)
    {
        return RunTimeResult().success_break();
    }

    RunTimeResult visit_BinaryOperationNode(const shared_ptr<BinaryOperationNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;
        const shared_ptr<DataType> left = res.register_result(visit(node->left_node, context));
        if (res.should_return())
            return res;
        const shared_ptr<DataType> right = res.register_result(visit(node->right_node, context));
        if (res.should_return())
            return res;

        shared_ptr<DataType> result = nullptr;
        shared_ptr<RunTimeError> error = nullptr;

        if (node->operator_token.type == T_PLUS)
            tie(result, error) = left->add(right);
        else if (node->operator_token.type == T_MINUS)
            tie(result, error) = left->subtract(right);
        else if (node->operator_token.type == T_MUL)
            tie(result, error) = left->multiply(right);
        else if (node->operator_token.type == T_DIVIDE)
            tie(result, error) = left->divide(right);
        else if (node->operator_token.type == T_MODULUS)
            tie(result, error) = left->modulus(right);
        else if (node->operator_token.type == T_FLOOR)
            tie(result, error) = left->floor_divide(right);
        else if (node->operator_token.type == T_EXP)
            tie(result, error) = left->exponent(right);
        else if (node->operator_token.type == T_EE)
            tie(result, error) = left->get_comparison_eq(right);
        else if (node->operator_token.type == T_NEQ)
            tie(result, error) = left->get_comparison_neq(right);
        else if (node->operator_token.type == T_LT)
            tie(result, error) = left->get_comparison_lt(right);
        else if (node->operator_token.type == T_GT)
            tie(result, error) = left->get_comparison_gt(right);
        else if (node->operator_token.type == T_LTE)
            tie(result, error) = left->get_comparison_lte(right);
        else if (node->operator_token.type == T_GTE)
            tie(result, error) = left->get_comparison_gte(right);
        else if (node->operator_token.type == T_KEYWORD)
        {
            if (any_cast<string>(node->operator_token.value) == "and")
                tie(result, error) = left->and_by(right);
            else if (any_cast<string>(node->operator_token.value) == "or")
                tie(result, error) = left->or_by(right);
        }

        if (error)
            return res.failure(*error);
        if (!result)
            return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "Unsupported operation", context));

        result->set_pos(node->pos_start, node->pos_end);
        return res.success(result);
    }

    RunTimeResult visit_TernaryOperationNode(const shared_ptr<TernaryOperationNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;
        auto comp_node = res.register_result(visit(node->comp_node, context));
        if (res.should_return())
            return res;

        auto [cond, error] = comp_node->is_true();
        if (error)
            return res.failure(*error);

        shared_ptr<DataType> result;
        if (cond && cond->is_truthy())
        {
            result = res.register_result(visit(node->true_node, context));
            if (res.should_return())
                return res;
        }
        else
        {
            result = res.register_result(visit(node->false_node, context));
            if (res.should_return())
                return res;
        }

        result->set_pos(node->pos_start, node->pos_end);
        return res.success(result);
    }

    RunTimeResult visit_UnaryOperationNode(const shared_ptr<UnaryOperationNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;
        auto number = res.register_result(visit(node->node, context));
        if (res.should_return())
            return res;

        shared_ptr<DataType> result = nullptr;
        shared_ptr<RunTimeError> error = nullptr;

        if (node->operator_token.type == T_MINUS)
        {
            tie(result, error) = number->multiply(std::static_pointer_cast<DataType>(make_shared<Number>(-1LL)));
        }
        else if (node->operator_token.type == T_KEYWORD && any_cast<string>(node->operator_token.value) == "not")
        {
            tie(result, error) = number->not_by();
        }

        if (error)
            return res.failure(*error);
        if (!result)
            return res.failure(RunTimeError(node->pos_start.value_or(Position()), node->pos_end.value_or(Position()), "Unsupported unary operation", context));

        result->set_pos(node->pos_start, node->pos_end);
        return res.success(result);
    }

    RunTimeResult visit_DictNode(const shared_ptr<DictNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;
        vector<pair<shared_ptr<DataType>, shared_ptr<DataType>>> elements;

        for (const auto &pair : node->keyval_nodes)
        {
            auto key = res.register_result(visit(pair.first, context));
            if (res.should_return())
                return res;

            if (!dynamic_cast<Number *>(key.get()) && !dynamic_cast<String *>(key.get()))
            {
                return res.failure(IllegalOperationError(pair.first->pos_start.value_or(Position{}), pair.first->pos_end.value_or(Position{}), "Dictionary keys must be numbers or strings", context));
            }

            auto value = res.register_result(visit(pair.second, context));
            if (res.should_return())
                return res;

            elements.push_back({key, value});
        }

        auto dict_val = make_shared<Dict>(elements);
        dict_val->set_context(context);
        dict_val->set_pos(node->pos_start.value_or(Position{}), node->pos_end.value_or(Position{}));

        return res.success(dict_val);
    }

    RunTimeResult visit_TryNode(const shared_ptr<TryNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;
        auto try_result = res.register_result(visit(node->body_node, context));

        if (!res.error)
        {
            if (node->clean_node)
            {
                res.register_result(visit(node->clean_node->body_node, context));
                if (res.should_return())
                    return res;
            }
            return res.success(try_result);
        }

        auto error = res.error;
        res.error = nullptr;
        bool handled = false;

        for (const auto &trap_node : node->trap_nodes)
        {
            bool matches = false;
            if (!trap_node->error_type)
            {
                matches = true;
            }
            else
            {
                string caught_err = any_cast<string>(trap_node->error_type->value);
                if (caught_err == "RunTimeError" || caught_err == error->error_name)
                {
                    matches = true;
                }
                else if (find(ERROR_TYPES.begin(), ERROR_TYPES.end(), caught_err) == ERROR_TYPES.end())
                {
                    return res.failure(InvalidErrorTypeError(trap_node->pos_start.value_or(Position{}), trap_node->pos_end.value_or(Position{}), "'" + caught_err + "' is not a valid error type", context));
                }
            }

            if (matches)
            {
                auto trap_context = make_shared<Context>("<trap block>", context, trap_node->pos_start.value_or(Position{}));
                trap_context->symbol_table = make_shared<SymbolTable>(context->symbol_table);

                if (trap_node->error_name)
                {
                    auto err_str = make_shared<String>(error->to_string());
                    err_str->set_pos(trap_node->pos_start.value_or(Position{}), trap_node->pos_end.value_or(Position{}));
                    err_str->set_context(trap_context);
                    trap_context->symbol_table->set(any_cast<string>(trap_node->error_name->value), err_str);
                }

                res.register_result(visit(trap_node->body_node, trap_context));
                if (res.error)
                    return res;
                handled = true;
                break;
            }
        }

        if (node->clean_node)
        {
            res.register_result(visit(node->clean_node->body_node, context));
            if (res.should_return())
                return res;
        }

        if (handled)
            return res.success(nullptr);

        return res.failure(*error);
    }

    RunTimeResult visit_ModelNode(const shared_ptr<ModelNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;

        const string &model_name = any_cast<string>(node->name_tok.value);

        vector<shared_ptr<ModelType>> parents;
        for (const auto &parent_tok : node->parent_name_toks)
        {
            const string &pname = any_cast<string>(parent_tok.value);
            auto parent_val = context->symbol_table->get(pname);
            if (!parent_val)
            {
                return res.failure(NameError(
                    parent_tok.pos_start.value_or(Position()), parent_tok.pos_end.value_or(Position()),
                    "Parent class '" + pname + "' is not defined", context));
            }
            auto parent_model = dynamic_pointer_cast<ModelType>(parent_val);
            if (!parent_model)
            {
                return res.failure(TypeError(
                    parent_tok.pos_start.value_or(Position()), parent_tok.pos_end.value_or(Position()),
                    "'" + pname + "' is not a class and cannot be inherited from", context));
            }
            parents.push_back(parent_model);
        }

        vector<AttrInfo> own_attributes;
        vector<shared_ptr<Node>> attr_nodes;
        shared_ptr<Node> init_node = nullptr;
        unordered_map<string, MethodInfo> method_nodes;

        for (const auto &member : node->body_nodes)
        {
            if (auto *func = dynamic_cast<FunctionDefinitionNode *>(member.get()))
            {
                string mname = func->var_name_tok.has_value()
                                   ? any_cast<string>(func->var_name_tok->value)
                                   : "";
                if (!mname.empty())
                {
                    method_nodes[mname] = MethodInfo{member, func->access_modifier};
                }
            }
            else if (dynamic_cast<InitNode *>(member.get()))
            {
                init_node = member;
            }
            else if (auto *an = dynamic_cast<AttrNode *>(member.get()))
            {
                attr_nodes.push_back(member);
                for (auto &[name_tok, default_node] : an->declarations)
                {
                    own_attributes.push_back(AttrInfo{
                        any_cast<string>(name_tok.value),
                        default_node,
                        an->access_modifier
                    });
                }
            }
        }

        auto model = make_shared<ModelType>(model_name, own_attributes, attr_nodes,
                                            init_node, method_nodes, parents);
        model->set_context(context).set_pos(node->pos_start, node->pos_end);

        context->symbol_table->set(model_name, model);
        return res.success(model);
    }

    RunTimeResult visit_AttrAccessNode(const shared_ptr<AttrAccessNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;

        auto object_val = res.register_result(visit(node->object_node, context));
        if (res.should_return())
            return res;

        const string &attr_name = any_cast<string>(node->attr_name_tok.value);

        shared_ptr<DataType> value;
        if (auto inst = dynamic_pointer_cast<ModelInstance>(object_val))
        {
            auto [v, err] = inst->get_attr(attr_name, *this, context);
            if (err)
                return res.failure(*err);
            value = v;
        }
        else
        {
            return res.failure(AttributeError(
                node->pos_start.value_or(Position()), node->pos_end.value_or(Position()),
                "Cannot access attribute '" + attr_name + "' on this type",
                context));
        }

        if (!value)
        {
            return res.failure(AttributeError(
                node->pos_start.value_or(Position()), node->pos_end.value_or(Position()),
                "Attribute '" + attr_name + "' not found",
                context));
        }

        value->set_pos(node->pos_start, node->pos_end).set_context(context);
        return res.success(value);
    }

    RunTimeResult visit_AttrAssignNode(const shared_ptr<AttrAssignNode> &node, const shared_ptr<Context> &context)
    {
        RunTimeResult res;

        auto object_val = res.register_result(visit(node->object_node, context));
        if (res.should_return())
            return res;

        auto value = res.register_result(visit(node->value_node, context));
        if (res.should_return())
            return res;

        const string &attr_name = any_cast<string>(node->attr_name_tok.value);

        if (auto inst = dynamic_pointer_cast<ModelInstance>(object_val))
        {
            inst->set_attr(attr_name, value);
            return res.success(value);
        }

        return res.failure(AttributeError(
            node->pos_start.value_or(Position()), node->pos_end.value_or(Position()),
            "Cannot assign attribute '" + attr_name + "' on this type",
            context));
    }
};
