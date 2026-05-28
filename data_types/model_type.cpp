#include "model_type.h"
#include "function_type.h"
#include "number_type.h"
#include "../language_core/interpreter.h"
#include "../language_core/symbol_table.h"
#include "../language_core/context.h"
#include "../ast_nodes/class_nodes.h"
#include "../ast_nodes/function_nodes.h"
#include "../ast_nodes/list_nodes.h"

// ── ModelType::execute ────────────────────────────────────────────────────────
// Instantiates a ModelInstance: initialises all attributes (from all ancestors),
// then runs the init constructor with the supplied args.
RunTimeResult ModelType::execute(const vector<shared_ptr<DataType>> &args, Interpreter &interp)
{
    RunTimeResult res;

    auto instance = make_shared<ModelInstance>(
        static_pointer_cast<ModelType>(shared_from_this()));
    instance->set_context(context).set_pos(pos_start, pos_end);
    instance->symbol_table->parent = context->symbol_table;

    auto all_attrs = all_attr_nodes();

    for (const auto &attr_n : all_attrs)
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
            exec_context->owner_class = static_pointer_cast<ModelType>(shared_from_this());

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

DataType::OperationResult ModelInstance::get_attr(const string &attr_name,
                                                   Interpreter &interp,
                                                   const shared_ptr<Context> &calling_context) const
{
    auto value = symbol_table->get(attr_name);
    if (value)
    {
        const AttrInfo *ai = model->find_attribute(attr_name);
        if (ai)
        {
            const string &access = ai->access_modifier;

            if (access == "secret")
            {
                // Only accessible from within the exact class that declared it
                auto attr_owner = model->find_attribute_owner(attr_name);
                auto this_val = calling_context->symbol_table->get("this");
                auto caller_inst = dynamic_pointer_cast<ModelInstance>(this_val);
                if (!caller_inst || caller_inst->model != attr_owner)
                {
                    return {nullptr, AttributeError(
                        pos_start.value_or(Position()), pos_end.value_or(Position()),
                        "Cannot access secret attribute '" + attr_name + "'",
                        calling_context)};
                }
            }
            else if (access == "guarded")
            {
                // Accessible from the declaring class or any subclass
                auto attr_owner = model->find_attribute_owner(attr_name);
                auto caller_class = calling_context->owner_class;
                if (!caller_class || !caller_class->is_descendant_of(attr_owner))
                {
                    return {nullptr, AttributeError(
                        pos_start.value_or(Position()), pos_end.value_or(Position()),
                        "Cannot access guarded attribute '" + attr_name + "'",
                        calling_context)};
                }
            }
            // "open" or "" → always accessible
        }

        return {value, nullopt};
    }

    const MethodInfo *mi = model->find_method(attr_name);
    if (mi)
    {
        const string &access = mi->access_modifier;

        if (access == "secret")
        {
            auto method_owner = model->find_method_owner(attr_name);
            auto caller_class = calling_context->owner_class;
            if (!caller_class || caller_class != method_owner)
            {
                return {nullptr, AttributeError(
                    pos_start.value_or(Position()), pos_end.value_or(Position()),
                    "Cannot access secret method '" + attr_name + "'",
                    calling_context)};
            }
        }
        else if (access == "guarded")
        {
            auto method_owner = model->find_method_owner(attr_name);
            auto caller_class = calling_context->owner_class;
            if (!caller_class || !caller_class->is_descendant_of(method_owner))
            {
                return {nullptr, AttributeError(
                    pos_start.value_or(Position()), pos_end.value_or(Position()),
                    "Cannot access guarded method '" + attr_name + "'",
                    calling_context)};
            }
        }

        auto *func_def = dynamic_cast<FunctionDefinitionNode *>(mi->node.get());
        if (func_def)
        {
            vector<string> arg_names;
            for (const auto &tok : func_def->arg_name_toks)
            {
                arg_names.push_back(any_cast<string>(tok.value));
            }

            auto self_ptr = const_cast<ModelInstance *>(this)->shared_from_this();

            auto method_owner = model->find_method_owner(attr_name);

            auto method = make_shared<Function>(
                attr_name,
                func_def->body_node,
                arg_names,
                false,
                self_ptr);
            method->set_context(context).set_pos(pos_start, pos_end);
            method->access_modifier_owner = method_owner;
            return {method, nullopt};
        }
    }

    return {nullptr, AttributeError(
                         pos_start.value_or(Position()), pos_end.value_or(Position()),
                         "'" + model->name + "' instance has no attribute '" + attr_name + "'",
                         context)};
}
