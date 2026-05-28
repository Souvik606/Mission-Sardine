#include "function_type.h"
#include "number_type.h"
#include "../language_core/interpreter.h"
#include "../language_core/symbol_table.h"

Function::Function(string name, shared_ptr<Node> body, vector<string> args, bool return_null,
                   shared_ptr<DataType> instance)
    : name(std::move(name)),
      body_node(std::move(body)),
      arg_names(std::move(args)),
      return_null(return_null),
      instance(std::move(instance))
{
    if (this->name.empty())
    {
        this->name = "<anonymous>";
    }
}

RunTimeResult Function::execute(const vector<shared_ptr<DataType>> &args, Interpreter &interpreter)
{
    RunTimeResult res;

    shared_ptr<Context> exec_context;

    if (instance)
    {
        exec_context = make_shared<Context>(this->name, this->context, this->pos_start);

        auto inst_sym = interpreter.get_instance_symbol_table(instance);
        if (!inst_sym)
        {
            exec_context->symbol_table = make_shared<SymbolTable>(this->context->symbol_table);
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
        exec_context = make_shared<Context>(this->name, this->context, this->pos_start);
        const auto new_symbol_table = make_shared<SymbolTable>();
        new_symbol_table->parent = this->context->symbol_table;
        exec_context->symbol_table = new_symbol_table;
    }

    if (args.size() > this->arg_names.size())
    {
        return res.failure(RunTimeError(
            this->pos_start.value_or(Position()), this->pos_end.value_or(Position()),
            std::to_string(args.size() - this->arg_names.size()) + " too many args passed into '" + this->name + "'",
            this->context));
    }

    if (args.size() < this->arg_names.size())
    {
        return res.failure(RunTimeError(
            this->pos_start.value_or(Position()), this->pos_end.value_or(Position()),
            std::to_string(this->arg_names.size() - args.size()) + " too few args passed into '" + this->name + "'",
            this->context));
    }

    for (size_t i = 0; i < args.size(); ++i)
    {
        const string &arg_name = this->arg_names[i];
        const shared_ptr<DataType> &arg_value = args[i];
        arg_value->set_context(exec_context);
        exec_context->symbol_table->set(arg_name, arg_value);
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
    auto new_func = make_shared<Function>(this->name, this->body_node, this->arg_names, this->return_null, this->instance);
    new_func->set_pos(this->pos_start, this->pos_end);
    new_func->set_context(this->context);
    return std::static_pointer_cast<DataType>(new_func);
}

inline string Function::to_string() const
{
    return "<function " + this->name + ">";
}