#pragma once
#include "data_type.h"
#include "number_type.h"

class Null final : public DataType {
public:
    Null() = default;

    [[nodiscard]] string get_type_name() const override {
        return "Null";
    }

    [[nodiscard]] shared_ptr<DataType> copy() const override {
        static auto instance = make_shared<Null>();
        return instance;
    }

    [[nodiscard]] string to_string() const override {
        return "Null";
    }

    [[nodiscard]] bool is_truthy() const override {
        return false;
    }

    [[nodiscard]] OperationResult is_true() const override {
        return { Number::make_bool(false), nullptr };
    }

    [[nodiscard]] OperationResult add(const shared_ptr<DataType>& other) const override { return illegal_op(other); }
    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType>& other) const override { return illegal_op(other); }
    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType>& other) const override { return illegal_op(other); }
    [[nodiscard]] OperationResult divide(const shared_ptr<DataType>& other) const override { return illegal_op(other); }
    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType>& other) const override { return illegal_op(other); }
    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType>& other) const override { return illegal_op(other); }
    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType>& other) const override { return illegal_op(other); }

    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType>& other) const override {
        bool eq = (dynamic_cast<const Null*>(other.get()) != nullptr);
        return { Number::make_bool(eq), nullptr };
    }

    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType>& other) const override {
        bool neq = (dynamic_cast<const Null*>(other.get()) == nullptr);
        return { Number::make_bool(neq), nullptr };
    }

    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType>& other) const override { return illegal_op(other); }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType>& other) const override { return illegal_op(other); }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType>& other) const override { return illegal_op(other); }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType>& other) const override { return illegal_op(other); }

    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType>& other) const override {
        return { Number::make_bool(false), nullptr };
    }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType>& other) const override {
        return { Number::make_bool(other->is_truthy()), nullptr };
    }
    [[nodiscard]] OperationResult not_by() const override {
        return { Number::make_bool(true), nullptr };
    }

    [[nodiscard]] OperationResult bitwise_and(const shared_ptr<DataType>& other) const override { return illegal_op(other); }
    [[nodiscard]] OperationResult bitwise_xor(const shared_ptr<DataType>& other) const override { return illegal_op(other); }
    [[nodiscard]] OperationResult bitwise_or(const shared_ptr<DataType>& other) const override { return illegal_op(other); }
    [[nodiscard]] OperationResult bitwise_not() const override {
        return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '~' to Null type", context) };
    }
    [[nodiscard]] OperationResult lshift(const shared_ptr<DataType>& other) const override { return illegal_op(other); }
    [[nodiscard]] OperationResult rshift(const shared_ptr<DataType>& other) const override { return illegal_op(other); }

private:
    [[nodiscard]] OperationResult illegal_op(const shared_ptr<DataType>& other) const {
        return { nullptr, make_shared<IllegalOperationError>(
            other->pos_start.value_or(Position()),
            other->pos_end.value_or(Position()),
            "Unsupported operand type(s) for Null",
            context
        ) };
    }
};
