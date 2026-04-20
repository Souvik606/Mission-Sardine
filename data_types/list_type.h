#pragma once
#include <bits/stdc++.h>
#include "data_type.h"
#include "number_type.h"
#include "string_type.h"
#include "../language_core/error.h"

using namespace std;

inline RunTimeError illegal_op_for_list(const DataType* self, const DataType* other = nullptr) {
    const auto pos_start = self->pos_start;
    const auto pos_end = other ? other->pos_end : self->pos_end;
    return RunTimeError(
        pos_start.value_or(Position()),
        pos_end.value_or(Position()),
        "Illegal Operation for type 'List'",
        self->context
    );
}

class List final : public DataType {
public:
    vector<shared_ptr<DataType>> elements;

    explicit List(vector<shared_ptr<DataType>> elements)
        : elements(std::move(elements)) {
        set_pos();
        set_context();
    }

    [[nodiscard]] shared_ptr<DataType> copy() const override {
        auto new_list = make_shared<List>(this->elements);
        new_list->set_pos(this->pos_start, this->pos_end);
        new_list->set_context(this->context);
        return std::static_pointer_cast<DataType>(new_list);
    }

    [[nodiscard]] string to_string() const override {
        stringstream ss;
        ss << "[";
        for (size_t i = 0; i < elements.size(); ++i) {
            if (elements[i]) ss << elements[i]->to_string();
            else ss << "null";

            if (i < elements.size() - 1) ss << ", ";
        }
        ss << "]";
        return ss.str();
    }

    [[nodiscard]] bool is_truthy() const override {
        return !elements.empty();
    }

    [[nodiscard]] OperationResult is_true() const override {
        auto result = make_shared<Number>(static_cast<long long>(this->elements.size()));
        result->set_context(this->context);
        result->set_pos(this->pos_start, this->pos_end);
        return std::make_pair(std::static_pointer_cast<DataType>(result), std::nullopt);
    }

    [[nodiscard]] OperationResult getByIndex(const vector<shared_ptr<DataType>>& indexes) const override {
        shared_ptr<DataType> temp = const_cast<List*>(this)->copy();

        for (const auto& idx : indexes) {
            auto num_idx = dynamic_cast<const Number*>(idx.get());
            if (!num_idx || holds_alternative<double>(num_idx->value)) {
                return std::make_pair(nullptr, RunTimeError(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Invalid Index Type", this->context));
            }
            long long i = get<long long>(num_idx->value);

            if (auto list_temp = dynamic_cast<List*>(temp.get())) {
                if (i < 0 || i >= list_temp->elements.size()) {
                    return std::make_pair(nullptr, RunTimeError(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
                }
                temp = list_temp->elements[i];
            } else if (auto str_temp = dynamic_cast<String*>(temp.get())) {
                if (i < 0 || i >= str_temp->value.length()) {
                    return std::make_pair(nullptr, RunTimeError(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
                }
                auto new_str = make_shared<String>(string(1, str_temp->value[i]));
                new_str->set_context(this->context);
                temp = new_str;
            } else {
                 return std::make_pair(nullptr, RunTimeError(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Can't index a data type which is not iterable", this->context));
            }
        }
        return std::make_pair(std::static_pointer_cast<DataType>(temp), std::nullopt);
    }

    [[nodiscard]] OperationResult assignIndex(const vector<shared_ptr<DataType>>& indexes, const shared_ptr<DataType>& val) const override {
        if (indexes.empty()) return std::make_pair(nullptr, RunTimeError(this->pos_start.value_or(Position()), this->pos_end.value_or(Position()), "Index out of bounds", this->context));

        auto new_list = make_shared<List>(this->elements);
        new_list->set_pos(this->pos_start, this->pos_end);
        new_list->set_context(this->context);

        shared_ptr<DataType> temp = new_list;

        try {
            for (size_t k = 0; k < indexes.size() - 1; ++k) {
                auto num_idx = dynamic_cast<const Number*>(indexes[k].get());
                if (!num_idx || holds_alternative<double>(num_idx->value)) {
                    return std::make_pair(nullptr, RunTimeError(indexes[k]->pos_start.value_or(Position()), indexes[k]->pos_end.value_or(Position()), "Invalid Index Type", this->context));
                }
                long long i = get<long long>(num_idx->value);

                if (auto list_temp = dynamic_cast<List*>(temp.get())) {
                    if (i < 0 || i >= list_temp->elements.size()) {
                        return std::make_pair(nullptr, RunTimeError(indexes[k]->pos_start.value_or(Position()), indexes[k]->pos_end.value_or(Position()), "Index out of bounds", this->context));
                    }

                    auto next_element = list_temp->elements[i];

                    if (auto next_as_list = dynamic_pointer_cast<List>(next_element)) {
                        auto copied_sublist = make_shared<List>(next_as_list->elements);
                        list_temp->elements[i] = copied_sublist;
                        temp = copied_sublist;
                    } else {
                        temp = next_element;
                    }
                } else if (dynamic_cast<String*>(temp.get())) {
                    return std::make_pair(nullptr, RunTimeError(indexes[k]->pos_start.value_or(Position()), indexes[k]->pos_end.value_or(Position()), "Can't assign inside string beyond one level", this->context));
                } else {
                    return std::make_pair(nullptr, RunTimeError(indexes[k]->pos_start.value_or(Position()), indexes[k]->pos_end.value_or(Position()), "Can't index a data type which is not iterable", this->context));
                }
            }

            auto last_idx = indexes.back();
            auto num_idx = dynamic_cast<const Number*>(last_idx.get());
            if (!num_idx || holds_alternative<double>(num_idx->value)) {
                return std::make_pair(nullptr, RunTimeError(last_idx->pos_start.value_or(Position()), last_idx->pos_end.value_or(Position()), "Invalid Index Type", this->context));
            }
            long long i = get<long long>(num_idx->value);

            if (auto list_temp = dynamic_cast<List*>(temp.get())) {
                if (i < 0 || i >= list_temp->elements.size()) {
                     return std::make_pair(nullptr, RunTimeError(last_idx->pos_start.value_or(Position()), last_idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
                }
                list_temp->elements[i] = val;
                return std::make_pair(std::static_pointer_cast<DataType>(new_list), std::nullopt);
            } else if (auto str_temp = dynamic_cast<String*>(temp.get())) {
                auto str_val = dynamic_cast<const String*>(val.get());
                if (!str_val || str_val->value.length() != 1) {
                    return std::make_pair(nullptr, RunTimeError(val->pos_start.value_or(Position()), val->pos_end.value_or(Position()), "Assigned value must be a single character string", this->context));
                }
                if (i < 0 || i >= str_temp->value.length()) {
                     return std::make_pair(nullptr, RunTimeError(last_idx->pos_start.value_or(Position()), last_idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
                }

                string s = str_temp->value;
                s[i] = str_val->value[0];
                auto replaced = make_shared<String>(s);
                replaced->set_context(this->context);

                shared_ptr<List> parent = new_list;
                for (size_t k = 0; k < indexes.size() - 2; ++k) {
                    long long p_i = get<long long>(dynamic_pointer_cast<Number>(indexes[k])->value);
                    parent = dynamic_pointer_cast<List>(parent->elements[p_i]);
                }
                long long p_i = get<long long>(dynamic_pointer_cast<Number>(indexes[indexes.size() - 2])->value);
                parent->elements[p_i] = replaced;

                return std::make_pair(std::static_pointer_cast<DataType>(new_list), std::nullopt);
            } else {
                return std::make_pair(nullptr, RunTimeError(last_idx->pos_start.value_or(Position()), last_idx->pos_end.value_or(Position()), "Can't index a data type which is not iterable", this->context));
            }
        } catch (...) {
            auto bad_idx = indexes.back();
            return std::make_pair(nullptr, RunTimeError(bad_idx->pos_start.value_or(Position()), bad_idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
        }
    }

    [[nodiscard]] OperationResult add(const shared_ptr<DataType>& operand) const override {
        auto new_list = make_shared<List>(this->elements);
        if (dynamic_cast<const Number*>(operand.get()) || dynamic_cast<const String*>(operand.get())) {
            new_list->elements.push_back(operand);
            new_list->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(new_list), std::nullopt);
        }
        if (const auto other_list = dynamic_cast<const List*>(operand.get())) {
            new_list->elements.insert(
                new_list->elements.end(),
                other_list->elements.begin(),
                other_list->elements.end()
            );
            new_list->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(new_list), std::nullopt);
        }
        return std::make_pair(nullptr, illegal_op_for_list(this, operand.get()));
    }

    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType>& operand) const override {
        if (const auto num = dynamic_cast<const Number*>(operand.get())) {
            if (const auto int_val = std::get_if<long long>(&num->value)) {
                auto new_list = make_shared<List>(this->elements);
                try {
                    if (*int_val < 0 || *int_val >= new_list->elements.size()) {
                        throw std::out_of_range("Index out of bounds");
                    }
                    new_list->elements.erase(new_list->elements.begin() + *int_val);
                    new_list->set_context(this->context);
                    return std::make_pair(std::static_pointer_cast<DataType>(new_list), std::nullopt);
                }
                catch (...) {
                    return std::make_pair(nullptr, RunTimeError(num->pos_start.value_or(Position()), num->pos_end.value_or(Position()), "Index out of bounds", this->context));
                }
            }
            return std::make_pair(nullptr, RunTimeError(num->pos_start.value_or(Position()), num->pos_end.value_or(Position()), "Index must be of an integer Number type", this->context));
        }
        return std::make_pair(nullptr, illegal_op_for_list(this, operand.get()));
    }

    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType>& operand) const override {
        if (const auto num = dynamic_cast<const Number*>(operand.get())) {
            if (const auto int_val = std::get_if<long long>(&num->value)) {
                if (*int_val < 0) {
                     return std::make_pair(nullptr, RunTimeError(num->pos_start.value_or(Position()), num->pos_end.value_or(Position()), "List repetition cannot be negative", this->context));
                }
                vector<shared_ptr<DataType>> new_elements;
                for (long long i = 0; i < *int_val; ++i) {
                    new_elements.insert(new_elements.end(), this->elements.begin(), this->elements.end());
                }
                auto new_list = make_shared<List>(new_elements);
                new_list->set_context(this->context);
                return std::make_pair(std::static_pointer_cast<DataType>(new_list), std::nullopt);
            }
            return std::make_pair(nullptr, RunTimeError(num->pos_start.value_or(Position()), num->pos_end.value_or(Position()), "Expected an integer Number type", this->context));
        }
        return std::make_pair(nullptr, illegal_op_for_list(this, operand.get()));
    }

    [[nodiscard]] OperationResult divide(const shared_ptr<DataType>& other) const override {
        auto new_list = make_shared<List>(this->elements);
        for (size_t i = 0; i < new_list->elements.size(); ++i) {
            auto el = new_list->elements[i];

            if (auto other_list = dynamic_cast<const List*>(other.get())) {
                if (auto el_list = dynamic_pointer_cast<List>(el)) {
                    auto [is_eq, error] = el_list->get_comparison_eq(other);
                    if (is_eq && is_eq->is_truthy()) {
                        new_list->elements.erase(new_list->elements.begin() + i);
                        break;
                    }
                }
            } else if (!dynamic_cast<List*>(el.get())) {
                auto [is_eq, error] = el->get_comparison_eq(other);
                if (is_eq && is_eq->is_truthy()) {
                    new_list->elements.erase(new_list->elements.begin() + i);
                    break;
                }
            }
        }
        return std::make_pair(std::static_pointer_cast<DataType>(new_list), std::nullopt);
    }

    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, RunTimeError(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '%' to a List", context)); }
    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, RunTimeError(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '//' to a List", context)); }
    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, RunTimeError(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '**' to a List", context)); }

    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType>& other) const override {
        if (const auto o = dynamic_cast<const List*>(other.get())) {
            if (this->elements.size() != o->elements.size()) {
                auto res = make_shared<Number>(0LL);
                res->set_context(this->context);
                return std::make_pair(std::static_pointer_cast<DataType>(res), std::nullopt);
            }

            bool all_match = true;
            for (size_t i = 0; i < this->elements.size(); ++i) {
                auto [is_eq, error] = this->elements[i]->get_comparison_eq(o->elements[i]);
                if (error || !is_eq || !is_eq->is_truthy()) {
                    all_match = false;
                    break;
                }
            }

            auto res = make_shared<Number>(static_cast<long long>(all_match));
            res->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(res), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a List", context));
    }

    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType>& other) const override {
        auto [eq_res, error] = get_comparison_eq(other);
        if (error) return std::make_pair(nullptr, error);

        auto res = make_shared<Number>(static_cast<long long>(!eq_res->is_truthy()));
        res->set_context(this->context);
        return std::make_pair(std::static_pointer_cast<DataType>(res), std::nullopt);
    }

    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, RunTimeError(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '<' to a List", context)); }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, RunTimeError(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '>' to a List", context)); }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, RunTimeError(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '<=' to a List", context)); }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, RunTimeError(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '>=' to a List", context)); }
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, RunTimeError(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply 'and' to a List", context)); }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, RunTimeError(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply 'or' to a List", context)); }
    [[nodiscard]] OperationResult not_by() const override { return std::make_pair(nullptr, RunTimeError(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply 'not' to a List", context)); }
};