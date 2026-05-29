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

inline shared_ptr<RunTimeError> illegal_op_error(const DataType *self, const DataType *other)
{
    return make_shared<RunTimeError>(
        self->pos_start.value_or(Position()),
        other->pos_end.value_or(Position()),
        "Illegal Operation",
        self->context);
}

inline shared_ptr<RunTimeError> illegal_op_error(const DataType *self)
{
    return make_shared<RunTimeError>(
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
    vector<pair<string, shared_ptr<Node>>> arg_nodes;
    bool return_null;
    shared_ptr<DataType> instance;
    shared_ptr<ModelType> access_modifier_owner; 

    explicit Function(string name, shared_ptr<Node> body, vector<pair<string, shared_ptr<Node>>> args, bool return_null,
                      shared_ptr<DataType> instance = nullptr);

    RunTimeResult execute(const vector<shared_ptr<DataType>> &pos_args, const map<string, shared_ptr<DataType>> &kw_args, Interpreter &interpreter);

    [[nodiscard]] bool is_truthy() const override
    {
        return true;
    }

    [[nodiscard]] OperationResult is_true() const override
    {
        auto result = make_shared<Number>(1LL);
        result->set_context(this->context);
        result->set_pos(this->pos_start, this->pos_end);
        return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
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

    [[nodiscard]] OperationResult bitwise_and(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult bitwise_xor(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult bitwise_or(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult bitwise_not() const override { return std::make_pair(nullptr, illegal_op_error(this)); }
    [[nodiscard]] OperationResult lshift(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
    [[nodiscard]] OperationResult rshift(const shared_ptr<DataType> &other) const override { return std::make_pair(nullptr, illegal_op_error(this, other.get())); }
};