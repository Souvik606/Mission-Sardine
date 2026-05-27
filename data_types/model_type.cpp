#include "model_type.h"
#include "function_type.h"
#include "number_type.h"
#include "../language_core/interpreter.h"
#include "../language_core/symbol_table.h"
#include "../ast_nodes/class_nodes.h"
#include "../ast_nodes/function_nodes.h"
#include "../ast_nodes/list_nodes.h"

RunTimeResult ModelType::execute(const vector<shared_ptr<DataType>> &args, Interpreter &interp)
{
    RunTimeResult res;

    auto instance = make_shared<ModelInstance>(
        static_pointer_cast<ModelType>(shared_from_this()));
    instance->set_context(context).set_pos(pos_start, pos_end);
    instance->symbol_table->parent = context->symbol_table;

    for (const auto &attr_n : attr_node_list)
    {
        auto *attr_node = dynamic_cast<AttrNode *>(attr_n.get());
        if (!attr_node)
            continue;
        for (auto &[name_tok, default_node] : attr_node->declarations)
        {
            const string &attr_name = any_cast<string>(name_tok.value);
            if (default_node)
            {
                auto default_val = res.register_result(interp.visit(default_node, context));
                if (res.should_return())
                    return res;
                instance->symbol_table->set(attr_name, default_val);
            }
            else
            {
                instance->symbol_table->set(attr_name, make_shared<Number>(0LL));
            }
        }
    }

    if (init_node)
    {
        auto *in = dynamic_cast<InitNode *>(init_node.get());
        if (in)
        {
            vector<string> param_names;
            for (const auto &tok : in->param_name_toks)
            {
                param_names.push_back(any_cast<string>(tok.value));
            }

            auto init_func = make_shared<Function>(
                "init",
                in->body_node,
                param_names,
                false,
                instance);
            init_func->set_context(context).set_pos(
                in->pos_start.has_value() ? in->pos_start.value() : Position(),
                in->pos_end.has_value() ? in->pos_end.value() : Position());

            auto exec_context = make_shared<Context>("init", context, pos_start);
            exec_context->symbol_table = instance->symbol_table;
            exec_context->symbol_table->set("this", instance);

            if (args.size() > param_names.size())
            {
                return res.failure(RunTimeError(
                    init_func->pos_start.value_or(Position()), init_func->pos_end.value_or(Position()),
                    std::to_string(args.size() - param_names.size()) + " too many args passed into 'init'",
                    context));
            }
            if (args.size() < param_names.size())
            {
                return res.failure(RunTimeError(
                    init_func->pos_start.value_or(Position()), init_func->pos_end.value_or(Position()),
                    std::to_string(param_names.size() - args.size()) + " too few args passed into 'init'",
                    context));
            }

            for (size_t i = 0; i < args.size(); ++i)
            {
                const string &arg_name = param_names[i];
                const shared_ptr<DataType> &arg_value = args[i];
                arg_value->set_context(exec_context);
                exec_context->symbol_table->set(arg_name, arg_value);
            }

            res.register_result(interp.visit(init_func->body_node, exec_context));
            if (res.should_return())
                return res;
        }
    }

    return res.success(instance);
}

DataType::OperationResult ModelInstance::get_attr(const string &attr_name, Interpreter &interp) const
{
    auto value = symbol_table->get(attr_name);
    if (value)
    {
        return {value, nullopt};
    }

    auto it = model->method_nodes.find(attr_name);
    if (it != model->method_nodes.end())
    {
        auto *func_def = dynamic_cast<FunctionDefinitionNode *>(it->second.get());
        if (func_def)
        {
            vector<string> arg_names;
            for (const auto &tok : func_def->arg_name_toks)
            {
                arg_names.push_back(any_cast<string>(tok.value));
            }

            auto self_ptr = const_cast<ModelInstance *>(this)->shared_from_this();
            auto method = make_shared<Function>(
                attr_name,
                func_def->body_node,
                arg_names,
                false,
                self_ptr);
            method->set_context(context).set_pos(pos_start, pos_end);
            return {method, nullopt};
        }
    }

    return {nullptr, AttributeError(
                         pos_start.value_or(Position()), pos_end.value_or(Position()),
                         "'" + model->name + "' instance has no attribute '" + attr_name + "'",
                         context)};
}
