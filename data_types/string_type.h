#pragma once
#include <bits/stdc++.h>
#include "data_type.h"
#include "number_type.h"
#include "../language_core/error.h"

using namespace std;

inline RunTimeError illegal_op_for_string(const DataType* self, const DataType* other = nullptr) {
    const auto pos_start = self->pos_start;
    const auto pos_end = other ? other->pos_end : self->pos_end;
    return RunTimeError(
        pos_start.value_or(Position()),
        pos_end.value_or(Position()),
        "Illegal Operation for type 'String'",
        self->context
    );
}

inline string repeat_string(const string& str, const long long n) {
    if (n < 0) return "";
    string result;
    result.reserve(str.size() * n);
    for (long long i = 0; i < n; ++i) {
        result += str;
    }
    return result;
}

class String final : public DataType {
public:
    string value;

    explicit String(string val) : value(std::move(val)) {}

    [[nodiscard]] shared_ptr<DataType> copy() const override {
        auto new_str = make_shared<String>(this->value);
        new_str->set_pos(this->pos_start, this->pos_end);
        new_str->set_context(this->context);
        return new_str;
    }

    [[nodiscard]] string to_string() const override {
        return "\"" + value + "\"";
    }

    [[nodiscard]] bool is_truthy() const override {
        return !value.empty();
    }

    [[nodiscard]] OperationResult add(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const String*>(operand.get())) {
            auto result = make_shared<String>(this->value + other->value);
            result->set_context(this->context);
            return {result, nullopt};
        }
        return {nullptr, illegal_op_for_string(this, operand.get())};
    }

    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            if (const auto int_val = std::get_if<long long>(&other->value)) {
                auto result = make_shared<String>(repeat_string(this->value, *int_val));
                result->set_context(this->context);
                return {result, nullopt};
            }
            return {nullptr, RunTimeError(pos_start.value_or(Position()), pos_end.value_or(Position()), "String can only be multiplied by an integer", context)};
        }
        return {nullptr, illegal_op_for_string(this, operand.get())};
    }

    // --- All other operations are illegal for String ---
    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_string(this, other.get())};
    }
    [[nodiscard]] OperationResult divide(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_string(this, other.get())};
    }
    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_string(this, other.get())};
    }
    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_string(this, other.get())};
    }
    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_string(this, other.get())};
    }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_string(this, other.get())};
    }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_string(this, other.get())};
    }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_string(this, other.get())};
    }
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_string(this, other.get())};
    }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_string(this, other.get())};
    }
    [[nodiscard]] OperationResult not_by() const override {
        return {nullptr, illegal_op_for_string(this)};
    }
};