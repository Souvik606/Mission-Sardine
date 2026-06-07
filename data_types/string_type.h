#pragma once
#include <bits/stdc++.h>
#include "data_type.h"
#include "number_type.h"
#include "../language_core/error.h"

using namespace std;

inline string repeat_string(const string& str, const long long n) {
    if (n < 0) return "";
    if (n > 0 && str.size() > 1000000 / n) {
        throw std::length_error("String length limit exceeded (max 1,000,000 characters)");
    }
    string result;
    result.reserve(str.size() * n);
    for (long long i = 0; i < n; ++i) {
        result += str;
    }
    return result;
}

class String final : public DataType, public enable_shared_from_this<String> {
public:
    string value;

    explicit String(string val) : value(std::move(val)) {}

    [[nodiscard]] string get_type_name() const override { return "String"; }
    [[nodiscard]] bool is_string() const override { return true; }

    [[nodiscard]] OperationResult get_attr(const string& attr_name, const shared_ptr<Context>& calling_context) const override;

    [[nodiscard]] shared_ptr<DataType> copy() const override {
        return const_cast<String*>(this)->shared_from_this();
    }

    [[nodiscard]] string to_string() const override {
        return "'" + value + "'";
    }

    [[nodiscard]] bool is_truthy() const override {
        return !value.empty();
    }

    [[nodiscard]] OperationResult is_true() const override {
        auto result = Number::make(static_cast<long long>(this->value.length()));
        result->set_context(this->context);
        result->set_pos(this->pos_start, this->pos_end);
        return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
    }

    [[nodiscard]] OperationResult getByIndex(const vector<shared_ptr<DataType>>& indexes, const Position& pos_start = Position(), const Position& pos_end = Position()) const override {
        if (indexes.size() == 1) {
            const auto& idx = indexes[0];
            if (idx->is_number()) {
                const auto num_idx = static_cast<const Number*>(idx.get());
                if (num_idx->is_float) {
                    return std::make_pair(nullptr, make_shared<IllegalOperationError>(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Invalid Index Type", this->context));
                }
                if (holds_alternative<double>(num_idx->value)) {
                    return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
                }
                long long i = std::get<long long>(num_idx->value);
                if (i < 0 || i >= static_cast<long long>(this->value.length())) {
                    return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
                }
                auto result = make_shared<String>(string(1, this->value[i]));
                result->set_context(this->context);
                return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
            }
            else {
                return std::make_pair(nullptr, make_shared<IllegalOperationError>(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Invalid Index Type", this->context));
            }
        }

        string temp = this->value;
        for (const auto& idx : indexes) {
            if (idx->is_number()) {
                const auto num_idx = static_cast<const Number*>(idx.get());
                if (num_idx->is_float) {
                    return std::make_pair(nullptr, make_shared<IllegalOperationError>(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Invalid Index Type", this->context));
                }
                if (holds_alternative<double>(num_idx->value)) {
                    return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
                }
                long long i = std::get<long long>(num_idx->value);
                if (i < 0 || i >= temp.length()) {
                    auto bad_idx = indexes.back();
                    return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(bad_idx->pos_start.value_or(Position()), bad_idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
                }
                temp = string(1, temp[i]);
            }
            else {
                return std::make_pair(nullptr, make_shared<IllegalOperationError>(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Invalid Index Type", this->context));
            }
        }
        auto result = make_shared<String>(temp);
        result->set_context(this->context);
        return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
    }

    [[nodiscard]] OperationResult assignIndex(const vector<shared_ptr<DataType>>& indexes, const shared_ptr<DataType>& val, const Position& pos_start = Position(), const Position& pos_end = Position()) const override {
        const String* str_val = val->is_string() ? static_cast<const String*>(val.get()) : nullptr;
        if (!str_val || str_val->value.length() != 1) {
            return std::make_pair(nullptr, make_shared<IllegalOperationError>(val->pos_start.value_or(Position()), val->pos_end.value_or(Position()), "Assigned value must be a single character string", this->context));
        }

        if (indexes.size() > 1) {
            auto idx = indexes[0];
            return std::make_pair(nullptr, make_shared<IllegalOperationError>(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Can't index beyond one dimension in string", this->context));
        }

        auto last_idx = indexes.back();
        const Number* num_idx = last_idx->is_number() ? static_cast<const Number*>(last_idx.get()) : nullptr;
        if (!num_idx || num_idx->is_float) {
            return std::make_pair(nullptr, make_shared<IllegalOperationError>(last_idx->pos_start.value_or(Position()), last_idx->pos_end.value_or(Position()), "Invalid Index Type", this->context));
        }

        if (holds_alternative<double>(num_idx->value)) {
            return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(last_idx->pos_start.value_or(Position()), last_idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
        }

        long long i = std::get<long long>(num_idx->value);
        if (i < 0 || i >= this->value.length()) {
            return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(last_idx->pos_start.value_or(Position()), last_idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
        }

        string new_str = this->value;
        new_str[i] = str_val->value[0];

        auto result = make_shared<String>(new_str);
        result->set_context(this->context);
        return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
    }

    [[nodiscard]] OperationResult add(const shared_ptr<DataType>& operand) const override {
        if (operand->is_string()) {
            const auto other = static_cast<const String*>(operand.get());
            if (this->value.length() + other->value.length() > 1000000) {
                return std::make_pair(nullptr, make_shared<ValueError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "String size limit exceeded (max 1,000,000 characters)", this->context));
            }
            auto result = make_shared<String>(this->value + other->value);
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
        }
        string hint = "";
        if (operand->is_number()) {
            const auto other_num = static_cast<const Number*>(operand.get());
            hint = "Cannot concatenate String and Number. Try converting with 'String(" + other_num->to_string() + ")' or wrap in an f-string.";
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a String type", this->context, hint));
    }

    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            if (const auto int_val = std::get_if<long long>(&other->value)) {
                if (*int_val < 0) {
                    return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "String repetition cannot be negative", this->context));
                }
                if (*int_val > 0 && this->value.length() > 1000000 / *int_val) {
                    return std::make_pair(nullptr, make_shared<ValueError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "String size limit exceeded (max 1,000,000 characters)", this->context));
                }
                auto result = make_shared<String>(repeat_string(this->value, *int_val));
                result->set_context(this->context);
                return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
            }
            return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected an integer Number type", this->context));
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected an integer Number type", this->context));
    }

    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '-' to a String type", context)); }
    [[nodiscard]] OperationResult divide(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '/' to a String type", context)); }
    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '%' to a String type", context)); }
    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '//' to a String type", context)); }
    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '**' to a String type", context)); }

    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType>& other) const override {
        if (other->is_string()) {
            const auto o = static_cast<const String*>(other.get());
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(this->value == o->value)), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType>& other) const override {
        if (other->is_string()) {
            const auto o = static_cast<const String*>(other.get());
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(this->value != o->value)), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType>& other) const override {
        if (other->is_string()) {
            const auto o = static_cast<const String*>(other.get());
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(this->value < o->value)), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType>& other) const override {
        if (other->is_string()) {
            const auto o = static_cast<const String*>(other.get());
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(this->value > o->value)), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType>& other) const override {
        if (other->is_string()) {
            const auto o = static_cast<const String*>(other.get());
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(this->value <= o->value)), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType>& other) const override {
        if (other->is_string()) {
            const auto o = static_cast<const String*>(other.get());
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(this->value >= o->value)), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType>& other) const override {
        if (other->is_string()) {
            const auto o = static_cast<const String*>(other.get());
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(is_truthy() && o->is_truthy())), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType>& other) const override {
        if (other->is_string()) {
            const auto o = static_cast<const String*>(other.get());
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(is_truthy() || o->is_truthy())), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult not_by() const override {
        return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(!is_truthy())), nullptr);
    }

    [[nodiscard]] OperationResult bitwise_and(const shared_ptr<DataType>& operand) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '&' to a String type", this->context)); }
    [[nodiscard]] OperationResult bitwise_xor(const shared_ptr<DataType>& operand) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '^' to a String type", this->context)); }
    [[nodiscard]] OperationResult bitwise_or(const shared_ptr<DataType>& operand) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '|' to a String type", this->context)); }
    [[nodiscard]] OperationResult bitwise_not() const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '~' to a String type", this->context)); }
    [[nodiscard]] OperationResult lshift(const shared_ptr<DataType>& operand) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '<<' to a String type", this->context)); }
    [[nodiscard]] OperationResult rshift(const shared_ptr<DataType>& operand) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '>>' to a String type", this->context)); }
};