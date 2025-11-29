#pragma once
#include "function_type.h"
#include "../language_core/interpreter.h"
#include "../language_core/symbol_table.h"

Function::Function(string name, shared_ptr<Node> body, vector<string> args, bool return_null)
    : name(std::move(name)),
    body_node(std::move(body)),
    arg_names(std::move(args)),
    return_null(return_null) {
    if (this->name.empty()) {
        this->name = "<anonymous>";
    }
}

RunTimeResult Function::execute(const vector<shared_ptr<DataType>>& args, Interpreter& interpreter) {
    RunTimeResult res;
    const auto new_context = make_shared<Context>(this->name, this->context, this->pos_start);

    const auto new_symbol_table = make_shared<SymbolTable>();
    new_symbol_table->parent = this->context->symbol_table;
    new_context->symbol_table = new_symbol_table;

    if (args.size() > this->arg_names.size()) {
        return res.failure(RunTimeError(
            this->pos_start.value_or(Position()), this->pos_end.value_or(Position()),
            std::to_string(args.size() - this->arg_names.size()) + " too many args passed into '" + this->name + "'",
            this->context
        ));
    }

    if (args.size() < this->arg_names.size()) {
        return res.failure(RunTimeError(
            this->pos_start.value_or(Position()), this->pos_end.value_or(Position()),
            std::to_string(this->arg_names.size() - args.size()) + " too few args passed into '" + this->name + "'",
            this->context
        ));
    }

    for (size_t i = 0; i < args.size(); ++i) {
        const string& arg_name = this->arg_names[i];
        const shared_ptr<DataType>& arg_value = args[i];
        arg_value->set_context(new_context);
        new_context->symbol_table->set(arg_name, arg_value);
    }

    const shared_ptr<DataType> value = res.register_result(interpreter.visit(this->body_node, new_context));
    if (res.error) return res;

    if (this->return_null) {
        auto null_value = make_shared<Number>(0LL);
        null_value->set_context(new_context).set_pos(this->pos_start, this->pos_end);
        return res.success(null_value);
    }

    return res.success(value);
}

shared_ptr<DataType> Function::copy() const {
    auto new_func = make_shared<Function>(this->name, this->body_node, this->arg_names, this->return_null);
    new_func->set_pos(this->pos_start, this->pos_end);
    new_func->set_context(this->context);
    return new_func;
}

inline string Function::to_string() const {
    return "<function " + this->name + ">";
}