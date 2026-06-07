#pragma once
#include <string>
#include <memory>
#include "data_type.h"
#include "model_type.h"
#include "function_type.h"
#include "../language_core/error.h"
#include "../ast_nodes/function_nodes.h"

using namespace std;

class SuperProxy final : public DataType {
public:
    shared_ptr<ModelInstance> instance;
    shared_ptr<ModelType> owner_class;

    SuperProxy(shared_ptr<ModelInstance> inst, shared_ptr<ModelType> owner)
        : instance(std::move(inst)), owner_class(std::move(owner)) {
        if (instance) {
            pos_start = instance->pos_start;
            pos_end = instance->pos_end;
            context = instance->context;
        }
    }

    [[nodiscard]] bool is_truthy() const override {
        return true;
    }

    [[nodiscard]] bool is_super_proxy() const override {
        return true;
    }

    [[nodiscard]] OperationResult is_true() const override {
        auto result = Number::make(1LL);
        result->set_context(this->context);
        result->set_pos(this->pos_start, this->pos_end);
        return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
    }

    [[nodiscard]] shared_ptr<DataType> copy() const override {
        auto proxy = make_shared<SuperProxy>(instance, owner_class);
        proxy->set_pos(pos_start, pos_end);
        proxy->set_context(context);
        return proxy;
    }

    [[nodiscard]] string to_string() const override {
        return "<super of " + owner_class->name + ">";
    }

    [[nodiscard]] OperationResult get_attr(const string& name, const shared_ptr<Context>& calling_context) const override {
        for (const auto& parent : owner_class->parents) {
            const MethodInfo* mi = parent->find_method(name);
            if (mi) {
                auto *func_def = dynamic_cast<FunctionDefinitionNode *>(mi->node.get());
                if (func_def) {
                    vector<pair<string, shared_ptr<Node>>> method_args;
                    for (const auto &p : func_def->arg_nodes) {
                        method_args.push_back({any_cast<string>(p.first.value), p.second});
                    }

                    auto method_owner = parent->find_method_owner(name);

                    auto method = make_shared<Function>(
                        name,
                        func_def->body_node,
                        method_args,
                        false,
                        instance);
                    method->set_context(instance->context).set_pos(func_def->pos_start, func_def->pos_end);
                    method->access_modifier_owner = method_owner;
                    return {method, nullptr};
                }
            }
        }

        return { nullptr, make_shared<AttributeError>(
            pos_start.value_or(Position()), pos_end.value_or(Position()),
            "No parent of '" + owner_class->name + "' defines method '" + name + "'",
            calling_context
        ) };
    }

    OperationResult set_attr(const string& name, shared_ptr<DataType> value) {
        return instance->set_attr(name, std::move(value));
    }

    // Binary / Unary operators (delegated error cases)
    [[nodiscard]] OperationResult add(const shared_ptr<DataType> &o) const override { return err("'+'"); }
    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType> &o) const override { return err("'-'"); }
    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType> &o) const override { return err("'*'"); }
    [[nodiscard]] OperationResult divide(const shared_ptr<DataType> &o) const override { return err("'/'"); }
    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType> &o) const override { return err("'%'"); }
    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType> &o) const override { return err("'**'"); }
    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType> &o) const override { return err("'//'"); }
    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType> &o) const override { return err("'=='"); }
    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType> &o) const override { return err("'!='"); }
    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType> &o) const override { return err("'<'"); }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType> &o) const override { return err("'>'"); }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType> &o) const override { return err("'<='"); }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType> &o) const override { return err("'>='"); }
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType> &o) const override { return err("'and'"); }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType> &o) const override { return err("'or'"); }
    [[nodiscard]] OperationResult not_by() const override { return err("'not'"); }

    [[nodiscard]] OperationResult bitwise_and(const shared_ptr<DataType> &o) const override { return err("'&'"); }
    [[nodiscard]] OperationResult bitwise_xor(const shared_ptr<DataType> &o) const override { return err("'^'"); }
    [[nodiscard]] OperationResult bitwise_or(const shared_ptr<DataType> &o) const override { return err("'|'"); }
    [[nodiscard]] OperationResult bitwise_not() const override { return err("'~'"); }
    [[nodiscard]] OperationResult lshift(const shared_ptr<DataType> &o) const override { return err("'<<'"); }
    [[nodiscard]] OperationResult rshift(const shared_ptr<DataType> &o) const override { return err("'>>'"); }

private:
    [[nodiscard]] OperationResult err(const string &op) const
    {
        return {nullptr, make_shared<IllegalOperationError>(
                             pos_start.value_or(Position()), pos_end.value_or(Position()),
                             "Cannot apply " + op + " to a super proxy", context)};
    }
};
