#pragma once

#include <bits/stdc++.h>
#include "data_type.h"
#include "number_type.h"
#include "../ast_results/runtime_result.h"

using namespace std;

using BuiltInFuncType = std::function<RunTimeResult(const vector<shared_ptr<DataType>>& args)>;

inline RunTimeError illegal_op_for_builtin(const DataType* self) {
    return RunTimeError(
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
        : name(std::move(name)), execute_impl(std::move(impl)) {}

    [[nodiscard]] RunTimeResult execute(const vector<shared_ptr<DataType>>& args) const {
        return this->execute_impl(args);
    }

    [[nodiscard]] bool is_truthy() const override {
        return true;
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

    [[nodiscard]] OperationResult add(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_builtin(this)};
    }
    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_builtin(this)};
    }
    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_builtin(this)};
    }
    [[nodiscard]] OperationResult divide(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_builtin(this)};
    }
    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_builtin(this)};
    }
    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_builtin(this)};
    }
    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_builtin(this)};
    }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_builtin(this)};
    }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_builtin(this)};
    }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_builtin(this)};
    }
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_builtin(this)};
    }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_builtin(this)};
    }
    [[nodiscard]] OperationResult not_by() const override {
        return {nullptr, illegal_op_for_builtin(this)};
    }
};