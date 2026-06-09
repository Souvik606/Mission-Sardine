#include "function_type.h"
#include "null_type.h"
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

class ContextPool {
private:
    vector<shared_ptr<Context>> pool;
public:
    static ContextPool& get() {
        thread_local ContextPool instance;
        return instance;
    }
    shared_ptr<Context> acquire(string display_name, shared_ptr<Context> parent, optional<Position> parent_entry_pos) {
        if (pool.empty()) {
            auto ctx = make_shared<Context>(std::move(display_name), parent, std::move(parent_entry_pos));
            ctx->symbol_table = make_shared<SymbolTable>();
            return ctx;
        }
        auto ctx = std::move(pool.back());
        pool.pop_back();
        ctx->reset(std::move(display_name), parent, std::move(parent_entry_pos));
        if (ctx->symbol_table) {
            ctx->symbol_table->reset(parent ? parent->symbol_table : nullptr);
        } else {
            ctx->symbol_table = make_shared<SymbolTable>(parent ? parent->symbol_table : nullptr);
        }
        return ctx;
    }
    void release(shared_ptr<Context> ctx) {
        if (!ctx) return;
        size_t closure_ref_count = 0;
        bool has_escaped = false;
        if (ctx->symbol_table) {
            for (const auto& [name, value] : ctx->symbol_table->get_symbols()) {
                if (value) {
                    if (auto* func = dynamic_cast<const Function*>(value.get())) {
                        if (func->closure_context == ctx) {
                            closure_ref_count++;
                            if (value.use_count() > 1) {
                                has_escaped = true;
                            }
                        }
                    }
                }
            }
        }
        if (!has_escaped && ctx.use_count() == closure_ref_count + 1) {
            if (ctx->symbol_table) {
                ctx->symbol_table->reset(nullptr);
            }
        }
        if (ctx.use_count() == 1 && (!ctx->symbol_table || ctx->symbol_table.use_count() == 1)) {
            pool.push_back(std::move(ctx));
        }
    }
};

RunTimeResult Function::execute(const vector<shared_ptr<DataType>> &pos_args, const map<string, shared_ptr<DataType>> &kw_args, Interpreter &interpreter, const shared_ptr<Context> &call_context)
{
    RunTimeResult res;

    shared_ptr<Context> exec_context;
    shared_ptr<Context> traceback_parent = call_context ? call_context : this->closure_context;

    if (instance)
    {
        traceback_parent = instance->context;
        exec_context = ContextPool::get().acquire("method " + this->name, traceback_parent, this->pos_start);

        auto inst_sym = interpreter.get_instance_symbol_table(instance);
        if (!inst_sym)
        {
            exec_context->symbol_table->reset(this->closure_context ? this->closure_context->symbol_table : nullptr);
        }
        else
        {
            exec_context->symbol_table->reset(inst_sym);
            exec_context->symbol_table->set("this", instance);
        }
        
        exec_context->owner_class = this->access_modifier_owner;
    }
    else
    {
        exec_context = ContextPool::get().acquire(this->name, traceback_parent, this->pos_start);
        exec_context->symbol_table->reset(this->closure_context ? this->closure_context->symbol_table : nullptr);
    }

    if (!UNBOUNDED_MODE && exec_context->depth > MAX_RECURSION_DEPTH)
    {
        return res.failure(StackDepthExceededError(
            this->pos_start.value_or(Position()), this->pos_end.value_or(Position()),
            "Maximum recursion depth exceeded (" + std::to_string(MAX_RECURSION_DEPTH) + ")",
            exec_context));
    }

    if (kw_args.empty() && pos_args.size() == this->arg_nodes.size())
    {
        for (size_t i = 0; i < pos_args.size(); ++i)
        {
            pos_args[i]->set_context(exec_context);
            exec_context->symbol_table->set(this->arg_nodes[i].first, pos_args[i]);
        }
    }
    else
    {
        unordered_map<string, shared_ptr<DataType>> final_args;

        if (pos_args.size() > this->arg_nodes.size())
        {
            return res.failure(ArgumentError(
                this->pos_start.value_or(Position()), this->pos_end.value_or(Position()),
                std::to_string(pos_args.size() - this->arg_nodes.size()) + " too many arguments passed into '" + this->name + "'",
                this->closure_context));
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
                    this->closure_context));
            }
            if (final_args.count(kw_name))
            {
                return res.failure(ArgumentError(
                    this->pos_start.value_or(Position()), this->pos_end.value_or(Position()),
                    "Multiple values for argument '" + kw_name + "' passed to '" + this->name + "'",
                    this->closure_context));
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
                        this->closure_context));
                }
            }
        }

        for (const auto &[name, value] : final_args)
        {
            value->set_context(exec_context);
            exec_context->symbol_table->set(name, value);
        }
    }

    shared_ptr<DataType> ret_val;
    {
        const shared_ptr<DataType> value = res.register_result(interpreter.visit(this->body_node, exec_context));
        if (!res.error)
        {
            if (res.func_return_value)
            {
                ret_val = res.func_return_value;
            }
            else if (this->return_null)
            {
                auto val = make_shared<Null>();
                val->set_context(exec_context).set_pos(this->pos_start, this->pos_end);
                ret_val = val;
            }
            else
            {
                ret_val = value;
            }
        }
    }

    if (ret_val)
    {
        ret_val->set_context(traceback_parent);
    }

    ContextPool::get().release(std::move(exec_context));

    if (res.error)
        return res;

    return res.success(ret_val);
}

shared_ptr<DataType> Function::copy() const
{
    auto new_func = make_shared<Function>(this->name, this->body_node, this->arg_nodes, this->return_null, this->instance);
    new_func->set_pos(this->pos_start, this->pos_end);
    new_func->set_context(this->closure_context);
    new_func->access_modifier_owner = this->access_modifier_owner;
    return std::static_pointer_cast<DataType>(new_func);
}

inline string Function::to_string() const
{
    return "<function " + this->name + ">";
}