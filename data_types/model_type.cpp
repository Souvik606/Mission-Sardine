#include "model_type.h"
#include "function_type.h"
#include "null_type.h"
#include "../language_core/interpreter.h"
#include "../language_core/symbol_table.h"
#include "../language_core/context.h"
#include "../ast_nodes/class_nodes.h"
#include "../ast_nodes/function_nodes.h"

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
                instance->symbol_table->set(attr_name, make_shared<Null>());
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
                    return {nullptr, make_shared<AttributeError>(
                        pos_start.value_or(Position()), pos_end.value_or(Position()),
                        "Cannot access secret attribute '" + attr_name + "'",
                        calling_context,
                        "'secret' attributes can only be accessed within the model that defines them.")};
                }
            }
            else if (access == "guarded")
            {
                // Accessible from the declaring class or any subclass
                auto attr_owner = model->find_attribute_owner(attr_name);
                auto caller_class = calling_context->owner_class;
                if (!caller_class || !caller_class->is_descendant_of(attr_owner))
                {
                    return {nullptr, make_shared<AttributeError>(
                        pos_start.value_or(Position()), pos_end.value_or(Position()),
                        "Cannot access guarded attribute '" + attr_name + "'",
                        calling_context,
                        "'guarded' attributes can only be accessed within the model or its subclasses.")};
                }
            }
            // "open" or "" → always accessible
        }

        return {value, nullptr};
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
                return {nullptr, make_shared<AttributeError>(
                    pos_start.value_or(Position()), pos_end.value_or(Position()),
                    "Cannot access secret method '" + attr_name + "'",
                    calling_context,
                    "'secret' methods can only be called from within the model that defines them.")};
            }
        }
        else if (access == "guarded")
        {
            auto method_owner = model->find_method_owner(attr_name);
            auto caller_class = calling_context->owner_class;
            if (!caller_class || !caller_class->is_descendant_of(method_owner))
            {
                return {nullptr, make_shared<AttributeError>(
                    pos_start.value_or(Position()), pos_end.value_or(Position()),
                    "Cannot access guarded method '" + attr_name + "'",
                    calling_context,
                    "'guarded' methods can only be called from within the model or its subclasses.")};
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
            method->set_context(context).set_pos(func_def->pos_start, func_def->pos_end);
            method->access_modifier_owner = method_owner;
            return {method, nullptr};
        }
    }

    return {nullptr, make_shared<AttributeError>(
                         pos_start.value_or(Position()), pos_end.value_or(Position()),
                         "'" + model->name + "' instance has no attribute '" + attr_name + "'",
                         context)};
}

DataType::OperationResult ModelInstance::err(const string &op) const
{
    return {nullptr, make_shared<IllegalOperationError>(
                         pos_start.value_or(Position()), pos_end.value_or(Position()),
                         "Cannot apply " + op + " to a model instance", context)};
}

static const unordered_map<string, string> BINARY_OP_METHODS = {
    {"add", "__add__"},
    {"subtract", "__sub__"},
    {"multiply", "__mul__"},
    {"divide", "__div__"},
    {"modulus", "__mod__"},
    {"floor_divide", "__floordiv__"},
    {"exponent", "__pow__"},
    {"bitwise_and", "__and__"},
    {"bitwise_or", "__or__"},
    {"bitwise_xor", "__xor__"},
    {"lshift", "__lshift__"},
    {"rshift", "__rshift__"},
    {"get_comparison_eq", "__eq__"},
    {"get_comparison_neq", "__neq__"},
    {"get_comparison_lt", "__lt__"},
    {"get_comparison_lte", "__lte__"},
    {"get_comparison_gt", "__gt__"},
    {"get_comparison_gte", "__gte__"},
    {"and_by", "__land__"},
    {"or_by", "__lor__"}
};

static const unordered_map<string, string> UNARY_OP_METHODS = {
    {"not_by", "__not__"},
    {"bitwise_not", "__bitnot__"}
};

static const unordered_map<string, string> OP_SYMBOLS = {
    {"add", "+"}, {"subtract", "-"}, {"multiply", "*"}, {"divide", "/"},
    {"modulus", "%"}, {"floor_divide", "//"}, {"exponent", "**"},
    {"bitwise_and", "&"}, {"bitwise_or", "|"}, {"bitwise_xor", "^"},
    {"lshift", "<<"}, {"rshift", ">>"},
    {"get_comparison_eq", "=="}, {"get_comparison_neq", "!="},
    {"get_comparison_lt", "<"},  {"get_comparison_lte", "<="},
    {"get_comparison_gt", ">"},  {"get_comparison_gte", ">="},
    {"and_by", "and"}, {"or_by", "or"},
    {"not_by", "not"}, {"bitwise_not", "~"}
};

DataType::OperationResult ModelInstance::_call_op_method(const string &method_name, const vector<shared_ptr<DataType>> &args) const
{
    const MethodInfo *mi = model->find_method(method_name);
    if (!mi)
    {
        return {nullptr, nullptr}; // not found
    }

    auto *func_def = dynamic_cast<FunctionDefinitionNode *>(mi->node.get());
    if (!func_def)
    {
        return {nullptr, nullptr};
    }

    vector<pair<string, shared_ptr<Node>>> method_args;
    for (const auto &p : func_def->arg_nodes)
    {
        method_args.push_back({any_cast<string>(p.first.value), p.second});
    }

    auto self_ptr = const_cast<ModelInstance *>(this)->shared_from_this();

    auto func = make_shared<Function>(
        method_name,
        func_def->body_node,
        method_args,
        false,
        self_ptr);
    func->set_context(context).set_pos(func_def->pos_start, func_def->pos_end);

    Interpreter interp;
    map<string, shared_ptr<DataType>> kw_args;
    RunTimeResult res = func->execute(args, kw_args, interp);
    if (res.error)
    {
        return {nullptr, make_shared<RunTimeError>(*res.error)};
    }
    return {res.value, nullptr};
}

DataType::OperationResult ModelInstance::_binary_op(const string &op_name, const shared_ptr<DataType> &other) const
{
    auto it = BINARY_OP_METHODS.find(op_name);
    if (it != BINARY_OP_METHODS.end())
    {
        auto [result, error] = _call_op_method(it->second, {other});
        if (error)
            return {nullptr, error};
        if (result)
            return {result, nullptr};
    }

    if (op_name == "get_comparison_eq")
    {
        if (dynamic_cast<const Null*>(other.get()) != nullptr) {
            return {Number::make_bool(false), nullptr};
        }
        bool eq = (this == other.get());
        return {Number::make_bool(eq), nullptr};
    }
    else if (op_name == "get_comparison_neq")
    {
        auto [eq_val, err] = get_comparison_eq(other);
        if (err)
            return {nullptr, err};
        auto eq_num = dynamic_pointer_cast<Number>(eq_val);
        bool neq = (eq_num && !eq_num->is_truthy());
        return {Number::make_bool(neq), nullptr};
    }

    string symbol = op_name;
    auto sym_it = OP_SYMBOLS.find(op_name);
    if (sym_it != OP_SYMBOLS.end())
        symbol = sym_it->second;

    string user_method = "?";
    if (it != BINARY_OP_METHODS.end())
        user_method = it->second;

    return {nullptr, make_shared<IllegalOperationError>(
                         pos_start.value_or(Position()), pos_end.value_or(Position()),
                         "Operator '" + symbol + "' is not defined for '" + model->name + "'. Define 'method " + user_method + "(other) {...}' inside the model to enable it.",
                         context)};
}

DataType::OperationResult ModelInstance::_unary_op(const string &op_name) const
{
    auto it = UNARY_OP_METHODS.find(op_name);
    if (it != UNARY_OP_METHODS.end())
    {
        auto [result, error] = _call_op_method(it->second, {});
        if (error)
            return {nullptr, error};
        if (result)
            return {result, nullptr};
    }

    string symbol = op_name;
    auto sym_it = OP_SYMBOLS.find(op_name);
    if (sym_it != OP_SYMBOLS.end())
        symbol = sym_it->second;

    string user_method = "?";
    if (it != UNARY_OP_METHODS.end())
        user_method = it->second;

    return {nullptr, make_shared<IllegalOperationError>(
                         pos_start.value_or(Position()), pos_end.value_or(Position()),
                         "Operator '" + symbol + "' is not defined for '" + model->name + "'. Define 'method " + user_method + "() {...}' inside the model to enable it.",
                         context)};
}

DataType::OperationResult ModelInstance::add(const shared_ptr<DataType> &o) const { return _binary_op("add", o); }
DataType::OperationResult ModelInstance::subtract(const shared_ptr<DataType> &o) const { return _binary_op("subtract", o); }
DataType::OperationResult ModelInstance::divide(const shared_ptr<DataType> &o) const { return _binary_op("divide", o); }
DataType::OperationResult ModelInstance::modulus(const shared_ptr<DataType> &o) const { return _binary_op("modulus", o); }
DataType::OperationResult ModelInstance::floor_divide(const shared_ptr<DataType> &o) const { return _binary_op("floor_divide", o); }
DataType::OperationResult ModelInstance::exponent(const shared_ptr<DataType> &o) const { return _binary_op("exponent", o); }
DataType::OperationResult ModelInstance::get_comparison_eq(const shared_ptr<DataType> &o) const { return _binary_op("get_comparison_eq", o); }
DataType::OperationResult ModelInstance::get_comparison_neq(const shared_ptr<DataType> &o) const { return _binary_op("get_comparison_neq", o); }
DataType::OperationResult ModelInstance::get_comparison_lt(const shared_ptr<DataType> &o) const { return _binary_op("get_comparison_lt", o); }
DataType::OperationResult ModelInstance::get_comparison_gt(const shared_ptr<DataType> &o) const { return _binary_op("get_comparison_gt", o); }
DataType::OperationResult ModelInstance::get_comparison_lte(const shared_ptr<DataType> &o) const { return _binary_op("get_comparison_lte", o); }
DataType::OperationResult ModelInstance::get_comparison_gte(const shared_ptr<DataType> &o) const { return _binary_op("get_comparison_gte", o); }
DataType::OperationResult ModelInstance::and_by(const shared_ptr<DataType> &o) const { return _binary_op("and_by", o); }
DataType::OperationResult ModelInstance::or_by(const shared_ptr<DataType> &o) const { return _binary_op("or_by", o); }
DataType::OperationResult ModelInstance::bitwise_and(const shared_ptr<DataType> &o) const { return _binary_op("bitwise_and", o); }
DataType::OperationResult ModelInstance::bitwise_xor(const shared_ptr<DataType> &o) const { return _binary_op("bitwise_xor", o); }
DataType::OperationResult ModelInstance::bitwise_or(const shared_ptr<DataType> &o) const { return _binary_op("bitwise_or", o); }
DataType::OperationResult ModelInstance::lshift(const shared_ptr<DataType> &o) const { return _binary_op("lshift", o); }
DataType::OperationResult ModelInstance::rshift(const shared_ptr<DataType> &o) const { return _binary_op("rshift", o); }

DataType::OperationResult ModelInstance::not_by() const { return _unary_op("not_by"); }
DataType::OperationResult ModelInstance::bitwise_not() const { return _unary_op("bitwise_not"); }

DataType::OperationResult ModelInstance::multiply(const shared_ptr<DataType> &o) const
{
    // Unary minus case: detect Number(-1) and try __neg__ first
    if (auto num = dynamic_pointer_cast<Number>(o))
    {
        if (holds_alternative<long long>(num->value) && get<long long>(num->value) == -1)
        {
            auto [result, error] = _call_op_method("__neg__", {});
            if (result)
                return {result, nullptr};
            if (error)
                return {nullptr, error};
        }
    }

    // Normal multiplication
    auto [result, error] = _call_op_method("__mul__", {o});
    if (result)
        return {result, nullptr};
    if (error)
        return {nullptr, error};

    return _binary_op("multiply", o);
}
