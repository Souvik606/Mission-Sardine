#include "function_type.h"
#include "number_type.h"
#include "../language_core/interpreter.h"
#include "../language_core/symbol_table.h"
#include "../language_core/constants.h"
#include "../language_core/error.h"
#include <map>
#include <unordered_map>

Function::Function(string name, shared_ptr<Node> body, vector<pair<string, shared_ptr<Node>>> args, bool return_null,
                   shared_ptr<DataType> instance)
    : name(std::move(name)),
      body_node(std::move(body)),
      arg_nodes(std::move(args)),
      return_null(return_null),
      instance(std::move(instance))
{
    if (this->name.empty())
    {
        this->name = "<anonymous>";
    }
}

RunTimeResult Function::execute(const vector<shared_ptr<DataType>> &pos_args, const map<string, shared_ptr<DataType>> &kw_args, Interpreter &interpreter, const shared_ptr<Context> &call_context)
{
    RunTimeResult res;

    shared_ptr<Context> exec_context;
    shared_ptr<Context> traceback_parent = call_context ? call_context : this->context;

    if (instance)
    {
        traceback_parent = instance->context;
        exec_context = make_shared<Context>("method " + this->name, traceback_parent, this->pos_start);

        auto inst_sym = interpreter.get_instance_symbol_table(instance);
        if (!inst_sym)
        {
            exec_context->symbol_table = make_shared<SymbolTable>(this->context ? this->context->symbol_table : nullptr);
        }
        else
        {
            exec_context->symbol_table = make_shared<SymbolTable>(inst_sym);
            exec_context->symbol_table->set("this", instance);
        }
        
        exec_context->owner_class = this->access_modifier_owner;
    }
    else
    {
        exec_context = make_shared<Context>(this->name, traceback_parent, this->pos_start);
        const auto new_symbol_table = make_shared<SymbolTable>();
        new_symbol_table->parent = this->context ? this->context->symbol_table : nullptr;
        exec_context->symbol_table = new_symbol_table;
    }

    if (exec_context->depth > MAX_RECURSION_DEPTH)
    {
        return res.failure(StackDepthExceededError(
            this->pos_start.value_or(Position()), this->pos_end.value_or(Position()),
            "Maximum recursion depth exceeded (" + std::to_string(MAX_RECURSION_DEPTH) + ")",
            exec_context));
    }

    // check_and_populate_args logic:
    unordered_map<string, shared_ptr<DataType>> final_args;

    if (pos_args.size() > this->arg_nodes.size())
    {
        return res.failure(ArgumentError(
            this->pos_start.value_or(Position()), this->pos_end.value_or(Position()),
            std::to_string(pos_args.size() - this->arg_nodes.size()) + " too many arguments passed into '" + this->name + "'",
            this->context));
    }

    for (size_t i = 0; i < pos_args.size(); ++i)
    {
        final_args[this->arg_nodes[i].first] = pos_args[i];
    }

    for (const auto &[kw_name, kw_value] : kw_args)
    {
        bool found = false;
        for (const auto &param : this->arg_nodes)
        {
            if (param.first == kw_name)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            return res.failure(ArgumentError(
                this->pos_start.value_or(Position()), this->pos_end.value_or(Position()),
                "Unexpected keyword argument '" + kw_name + "' passed to '" + this->name + "'",
                this->context));
        }
        if (final_args.count(kw_name))
        {
            return res.failure(ArgumentError(
                this->pos_start.value_or(Position()), this->pos_end.value_or(Position()),
                "Multiple values for argument '" + kw_name + "' passed to '" + this->name + "'",
                this->context));
        }
        final_args[kw_name] = kw_value;
    }

    for (const auto &[p_name, p_default_node] : this->arg_nodes)
    {
        if (!final_args.count(p_name))
        {
            if (p_default_node)
            {
                auto default_value = res.register_result(interpreter.visit(p_default_node, exec_context));
                if (res.should_return()) return res;
                final_args[p_name] = default_value;
            }
            else
            {
                return res.failure(ArgumentError(
                    this->pos_start.value_or(Position()), this->pos_end.value_or(Position()),
                    "Missing required argument '" + p_name + "' for function '" + this->name + "'",
                    this->context));
            }
        }
    }

    for (const auto &[name, value] : final_args)
    {
        value->set_context(exec_context);
        exec_context->symbol_table->set(name, value);
    }

    const shared_ptr<DataType> value = res.register_result(interpreter.visit(this->body_node, exec_context));
    if (res.error)
        return res;

    if (res.func_return_value)
        return res.success(res.func_return_value);

    if (this->return_null)
    {
        auto val = make_shared<Number>(0LL);
        val->set_context(exec_context).set_pos(this->pos_start, this->pos_end);
        return res.success(std::static_pointer_cast<DataType>(val));
    }

    return res.success(value);
}

shared_ptr<DataType> Function::copy() const
{
    auto new_func = make_shared<Function>(this->name, this->body_node, this->arg_nodes, this->return_null, this->instance);
    new_func->set_pos(this->pos_start, this->pos_end);
    new_func->set_context(this->context);
    new_func->access_modifier_owner = this->access_modifier_owner;
    return std::static_pointer_cast<DataType>(new_func);
}

inline string Function::to_string() const
{
    return "<function " + this->name + ">";
}