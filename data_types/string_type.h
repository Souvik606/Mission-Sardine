#pragma once
#include <bits/stdc++.h>
#include "data_type.h"
#include "number_type.h"
#include "../language_core/error.h"

using namespace std;

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
        return std::static_pointer_cast<DataType>(new_str);
    }

    [[nodiscard]] string to_string() const override {
        return "\"" + value + "\"";
    }

    [[nodiscard]] bool is_truthy() const override {
        return !value.empty();
    }

    [[nodiscard]] OperationResult is_true() const override {
        auto result = make_shared<Number>(static_cast<long long>(this->value.length()));
        result->set_context(this->context);
        result->set_pos(this->pos_start, this->pos_end);
        return std::make_pair(std::static_pointer_cast<DataType>(result), std::nullopt);
    }

    [[nodiscard]] OperationResult getByIndex(const vector<shared_ptr<DataType>>& indexes) const override {
        string temp = this->value;
        for (const auto& idx : indexes) {
            if (const auto num_idx = dynamic_cast<const Number*>(idx.get())) {
                if (holds_alternative<double>(num_idx->value)) {
                    return std::make_pair(nullptr, IllegalOperationError(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Invalid Index Type", this->context));
                }
                long long i = get<long long>(num_idx->value);
                if (i < 0 || i >= temp.length()) {
                    auto bad_idx = indexes.back();
                    return std::make_pair(nullptr, IndexOutOfBoundsError(bad_idx->pos_start.value_or(Position()), bad_idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
                }
                temp = string(1, temp[i]);
            }
            else {
                return std::make_pair(nullptr, IllegalOperationError(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Invalid Index Type", this->context));
            }
        }
        auto result = make_shared<String>(temp);
        result->set_context(this->context);
        return std::make_pair(std::static_pointer_cast<DataType>(result), std::nullopt);
    }

    [[nodiscard]] OperationResult assignIndex(const vector<shared_ptr<DataType>>& indexes, const shared_ptr<DataType>& val) const override {
        auto str_val = dynamic_cast<const String*>(val.get());
        if (!str_val || str_val->value.length() != 1) {
            return std::make_pair(nullptr, IllegalOperationError(val->pos_start.value_or(Position()), val->pos_end.value_or(Position()), "Assigned value must be a single character string", this->context));
        }

        if (indexes.size() > 1) {
            auto idx = indexes[0];
            return std::make_pair(nullptr, IllegalOperationError(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Can't index beyond one dimension in string", this->context));
        }

        auto last_idx = indexes.back();
        auto num_idx = dynamic_cast<const Number*>(last_idx.get());
        if (!num_idx || holds_alternative<double>(num_idx->value)) {
            return std::make_pair(nullptr, IllegalOperationError(last_idx->pos_start.value_or(Position()), last_idx->pos_end.value_or(Position()), "Invalid Index Type", this->context));
        }

        long long i = get<long long>(num_idx->value);
        if (i < 0 || i >= this->value.length()) {
            return std::make_pair(nullptr, IndexOutOfBoundsError(last_idx->pos_start.value_or(Position()), last_idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
        }

        string new_str = this->value;
        new_str[i] = str_val->value[0];

        auto result = make_shared<String>(new_str);
        result->set_context(this->context);
        return std::make_pair(std::static_pointer_cast<DataType>(result), std::nullopt);
    }

    [[nodiscard]] OperationResult add(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const String*>(operand.get())) {
            auto result = make_shared<String>(this->value + other->value);
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), std::nullopt);
        }
        return std::make_pair(nullptr, IllegalOperationError(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a String type", this->context));
    }

    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            if (const auto int_val = std::get_if<long long>(&other->value)) {
                auto result = make_shared<String>(repeat_string(this->value, *int_val));
                result->set_context(this->context);
                return std::make_pair(std::static_pointer_cast<DataType>(result), std::nullopt);
            }
            return std::make_pair(nullptr, IllegalOperationError(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected an integer Number type", this->context));
        }
        return std::make_pair(nullptr, IllegalOperationError(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected an integer Number type", this->context));
    }

    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, IllegalOperationError(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '-' to a String type", context)); }
    [[nodiscard]] OperationResult divide(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, IllegalOperationError(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '/' to a String type", context)); }
    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, IllegalOperationError(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '%' to a String type", context)); }
    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, IllegalOperationError(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '//' to a String type", context)); }
    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, IllegalOperationError(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '**' to a String type", context)); }

    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType>& other) const override {
        if (const auto o = dynamic_cast<const String*>(other.get())) return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(this->value == o->value))), std::nullopt);
        return std::make_pair(nullptr, IllegalOperationError(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType>& other) const override {
        if (const auto o = dynamic_cast<const String*>(other.get())) return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(this->value != o->value))), std::nullopt);
        return std::make_pair(nullptr, IllegalOperationError(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType>& other) const override {
        if (const auto o = dynamic_cast<const String*>(other.get())) return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(this->value < o->value))), std::nullopt);
        return std::make_pair(nullptr, IllegalOperationError(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType>& other) const override {
        if (const auto o = dynamic_cast<const String*>(other.get())) return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(this->value > o->value))), std::nullopt);
        return std::make_pair(nullptr, IllegalOperationError(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType>& other) const override {
        if (const auto o = dynamic_cast<const String*>(other.get())) return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(this->value <= o->value))), std::nullopt);
        return std::make_pair(nullptr, IllegalOperationError(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType>& other) const override {
        if (const auto o = dynamic_cast<const String*>(other.get())) return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(this->value >= o->value))), std::nullopt);
        return std::make_pair(nullptr, IllegalOperationError(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType>& other) const override {
        if (const auto o = dynamic_cast<const String*>(other.get())) {
            return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(is_truthy() && o->is_truthy()))), std::nullopt);
        }
        return std::make_pair(nullptr, IllegalOperationError(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType>& other) const override {
        if (const auto o = dynamic_cast<const String*>(other.get())) {
            return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(is_truthy() || o->is_truthy()))), std::nullopt);
        }
        return std::make_pair(nullptr, IllegalOperationError(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a String type", this->context));
    }
    [[nodiscard]] OperationResult not_by() const override {
        return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(!is_truthy()))), std::nullopt);
    }
};