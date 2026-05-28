#pragma once
#include <string>
#include <vector>
#include <memory>
#include "data_type.h"
#include "number_type.h"
#include "../ast_results/runtime_result.h"

class Node;
using namespace std;

class Interpreter;

inline RunTimeError illegal_op_error(const DataType *self, const DataType *other)
{
    return RunTimeError(
        self->pos_start.value_or(Position()),
        other->pos_end.value_or(Position()),
        "Illegal Operation",
        self->context);
}

inline RunTimeError illegal_op_error(const DataType *self)
{
    return RunTimeError(
        self->pos_start.value_or(Position()),
        self->pos_end.value_or(Position()),
        "Illegal Operation",
        self->context);
}

class ModelType;

class Function final : public DataType
{
public:
    string name;
    shared_ptr<Node> body_node;
    vector<string> arg_names;
    bool return_null;
    shared_ptr<DataType> instance;
    shared_ptr<ModelType> access_modifier_owner; 

    explicit Function(string name, shared_ptr<Node> body, vector<string> args, bool return_null,
                      shared_ptr<DataType> instance = nullptr);

    RunTimeResult execute(const vector<shared_ptr<DataType>> &args, Interpreter &interpreter);

    [[nodiscard]] bool is_truthy() const override
    {
        return true;
    }

    [[nodiscard]] OperationResult is_true() const override
    {
        auto result = make_shared<Number>(1LL);
        result->set_context(this->context);
        result->set_pos(this->pos_start, this->pos_end);
        return std::make_pair(std::static_pointer_cast<DataType>(result), std::nullopt);
    }

    [[nodiscard]] shared_ptr<DataType> copy() const override;
    [[nodiscard]] string to_string() const override;

    [[nodiscard]] OperationResult add(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult divide(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult not_by() const override { return std::make_pair(nullptr, illegal_op_error(this)); }
};