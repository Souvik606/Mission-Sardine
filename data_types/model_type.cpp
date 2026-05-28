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
RunTimeResult ModelType::execute(const vector<shared_ptr<DataType>> &pos_args, const map<string, shared_ptr<DataType>> &kw_args, Interpreter &interp)
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
            vector<pair<string, shared_ptr<Node>>> init_params;
            for (const auto &p : in->param_nodes)
            {
                init_params.push_back({any_cast<string>(p.first.value), p.second});
            }

            auto init_func = make_shared<Function>(
                "init",
                in->body_node,
                init_params,
                false,
                instance);
            init_func->set_context(context).set_pos(
                in->pos_start.has_value() ? in->pos_start.value() : Position(),
                in->pos_end.has_value() ? in->pos_end.value() : Position());

            res.register_result(init_func->execute(pos_args, kw_args, interp));
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
            vector<pair<string, shared_ptr<Node>>> method_args;
            for (const auto &p : func_def->arg_nodes)
            {
                method_args.push_back({any_cast<string>(p.first.value), p.second});
            }

            auto self_ptr = const_cast<ModelInstance *>(this)->shared_from_this();

            auto method_owner = model->find_method_owner(attr_name);

            auto method = make_shared<Function>(
                attr_name,
                func_def->body_node,
                method_args,
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
