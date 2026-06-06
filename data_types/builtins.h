#pragma once
#include <bits/stdc++.h>
#include "data_type.h"
#include "number_type.h"
#include "../ast_results/runtime_result.h"

using namespace std;

using BuiltInFuncType = std::function<RunTimeResult(const Position& pos_start, const Position& pos_end, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context)>;

inline shared_ptr<RunTimeError> illegal_op_for_builtin(const DataType* self) {
    return make_shared<RunTimeError>(
        self->pos_start.value_or(Position()),
        self->pos_end.value_or(Position()),
        "Illegal Operation for built-in function",
        self->context
    );
}

class BuiltInFunction final : public DataType {
public:
    string name;
    BuiltInFuncType execute_impl;
    explicit BuiltInFunction(string name, BuiltInFuncType impl)
        : name(std::move(name)), execute_impl(std::move(impl)) {
    }

    [[nodiscard]] string get_type_name() const override { return "BuiltInFunction"; }
    [[nodiscard]] bool is_callable_type() const override { return true; }

    [[nodiscard]] RunTimeResult execute(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) const {
        auto exec_context = make_shared<Context>(this->name, context, this->pos_start);
        exec_context->symbol_table = make_shared<SymbolTable>(context->symbol_table);
        return this->execute_impl(this->pos_start.value_or(Position()), this->pos_end.value_or(Position()), args, kw_args, exec_context);
    }

    [[nodiscard]] bool is_truthy() const override {
        return true;
    }

    [[nodiscard]] OperationResult is_true() const override {
        auto result = make_shared<Number>(1LL);
        result->set_context(this->context);
        result->set_pos(this->pos_start, this->pos_end);
        return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
    }

    [[nodiscard]] shared_ptr<DataType> copy() const override {
        auto new_func = make_shared<BuiltInFunction>(this->name, this->execute_impl);
        new_func->set_pos(this->pos_start, this->pos_end);
        new_func->set_context(this->context);
        return new_func;
    }

    [[nodiscard]] string to_string() const override {
        return "<built-in function " + this->name + ">";
    }

    [[nodiscard]] OperationResult add(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult divide(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType>& other) const override {
        if (other->get_type_name() == "Null") {
            return { Number::make_bool(false), nullptr };
        }
        bool eq = (this == other.get());
        return { Number::make_bool(eq), nullptr };
    }
    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType>& other) const override {
        if (other->get_type_name() == "Null") {
            return { Number::make_bool(true), nullptr };
        }
        bool neq = (this != other.get());
        return { Number::make_bool(neq), nullptr };
    }
    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult not_by() const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }

    [[nodiscard]] OperationResult bitwise_and(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult bitwise_xor(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult bitwise_or(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult bitwise_not() const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult lshift(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
    [[nodiscard]] OperationResult rshift(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, illegal_op_for_builtin(this)); }
};