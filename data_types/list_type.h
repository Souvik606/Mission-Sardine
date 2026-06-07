#pragma once
#include <bits/stdc++.h>
#include "data_type.h"
#include "number_type.h"
#include "string_type.h"
#include "../language_core/error.h"
#include "cow_containers.h"

using namespace std;



class List final : public DataType, public enable_shared_from_this<List> {
public:
    CowVector<shared_ptr<DataType>> elements;

    explicit List(vector<shared_ptr<DataType>> elements)
        : elements(std::move(elements)) {
        set_pos();
        set_context();
    }

    explicit List(CowVector<shared_ptr<DataType>> elements)
        : elements(std::move(elements)) {
        set_pos();
        set_context();
    }

    [[nodiscard]] bool is_mutable() const override { return true; }
    void detach() { elements.detach(); }

    [[nodiscard]] string get_type_name() const override { return "List"; }
    [[nodiscard]] bool is_list() const override { return true; }

    [[nodiscard]] OperationResult get_attr(const string& attr_name, const shared_ptr<Context>& calling_context) const override;

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
        auto result = Number::make(static_cast<long long>(this->elements.size()));
        result->set_context(this->context);
        result->set_pos(this->pos_start, this->pos_end);
        return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
    }

    [[nodiscard]] OperationResult getByIndex(const vector<shared_ptr<DataType>>& indexes, const Position& pos_start = Position(), const Position& pos_end = Position()) const override {
        // Fast path: single integer index into a List (by far the most common case)
        if (indexes.size() == 1) {
            if (const auto num_idx = dynamic_cast<const Number*>(indexes[0].get())) {
                if (num_idx->is_float) {
                    return std::make_pair(nullptr, make_shared<IllegalOperationError>(
                        indexes[0]->pos_start.value_or(Position()),
                        indexes[0]->pos_end.value_or(Position()),
                        "Invalid Index Type", this->context));
                }
                if (holds_alternative<double>(num_idx->value)) {
                    return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(
                        indexes[0]->pos_start.value_or(Position()),
                        indexes[0]->pos_end.value_or(Position()),
                        "Index out of bounds", this->context));
                }
                long long idx = get<long long>(num_idx->value);
                if (idx < 0 || idx >= static_cast<long long>(elements.size())) {
                    return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(
                        indexes[0]->pos_start.value_or(Position()),
                        indexes[0]->pos_end.value_or(Position()),
                        "Index out of bounds", this->context));
                }
                return std::make_pair(elements[idx], nullptr);
            }
        }

        // General path: multi-level indexing (no upfront copy)
        const DataType* cur = this;
        shared_ptr<DataType> cur_owned;   // keeps ownership when we step into a child

        try {
            for (const auto& idx : indexes) {
                if (cur->is_dict()) {
                    if (dynamic_cast<const Number*>(idx.get()) || dynamic_cast<const String*>(idx.get())) {
                        auto [next, error] = cur->getByIndex({ idx });
                        if (error) return std::make_pair(nullptr, error);
                        cur_owned = next;
                        cur = cur_owned.get();
                    }
                    else {
                        return std::make_pair(nullptr, make_shared<DictKeyError>(
                            idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()),
                            "Dictionary keys must be numbers or strings", this->context));
                    }
                }
                else if (const auto num_idx = dynamic_cast<const Number*>(idx.get())) {
                    if (num_idx->is_float) {
                        return std::make_pair(nullptr, make_shared<IllegalOperationError>(
                            idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()),
                            "Invalid Index Type", this->context));
                    }
                    if (holds_alternative<double>(num_idx->value)) {
                        return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(
                            idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()),
                            "Index out of bounds", this->context));
                    }
                    long long i = get<long long>(num_idx->value);
                    if (const auto list_cur = dynamic_cast<const List*>(cur)) {
                        cur_owned = list_cur->elements.at(i);
                        cur = cur_owned.get();
                    }
                    else if (const auto str_cur = dynamic_cast<const String*>(cur)) {
                        cur_owned = make_shared<String>(string(1, str_cur->value.at(i)));
                        cur_owned->set_context(this->context);
                        cur = cur_owned.get();
                    }
                    else {
                        return std::make_pair(nullptr, make_shared<IllegalOperationError>(
                            idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()),
                            "Can't index a data type which is not iterable", this->context));
                    }
                }
                else {
                    return std::make_pair(nullptr, make_shared<IllegalOperationError>(
                        idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()),
                        "Invalid Index Type", this->context));
                }
            }
            return std::make_pair(cur_owned, nullptr);
        }
        catch (const out_of_range&) {
            auto bad_idx = indexes.back();
            return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(
                bad_idx->pos_start.value_or(Position()), bad_idx->pos_end.value_or(Position()),
                "Index out of bounds", this->context));
        }
    }

    [[nodiscard]] OperationResult assignIndex(const vector<shared_ptr<DataType>>& indexes, const shared_ptr<DataType>& val, const Position& pos_start = Position(), const Position& pos_end = Position()) const override {
        if (indexes.empty()) return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(this->pos_start.value_or(Position()), this->pos_end.value_or(Position()), "Index out of bounds", this->context));

        auto new_list = make_shared<List>(this->elements);
        new_list->set_pos(this->pos_start, this->pos_end);
        new_list->set_context(this->context);

        try {
            vector<shared_ptr<DataType>> parent_chain;
            vector<shared_ptr<DataType>> parent_indexes;
            shared_ptr<DataType> current = new_list;

            for (size_t i = 0; i + 1 < indexes.size(); ++i) {
                parent_chain.push_back(current);
                parent_indexes.push_back(indexes[i]);

                const auto& idx = indexes[i];
                if (current->is_dict()) {
                    if (dynamic_cast<const Number*>(idx.get()) || dynamic_cast<const String*>(idx.get())) {
                        auto [next_current, error] = current->getByIndex({ idx });
                        if (error) return std::make_pair(nullptr, error);
                        current = next_current;
                    }
                    else {
                        return std::make_pair(nullptr, make_shared<DictKeyError>(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Dictionary keys must be numbers or strings", this->context));
                    }
                }
                else if (const auto num_idx = dynamic_cast<const Number*>(idx.get())) {
                    if (num_idx->is_float) {
                        return std::make_pair(nullptr, make_shared<IllegalOperationError>(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Invalid Index Type", this->context));
                    }
                    if (holds_alternative<double>(num_idx->value)) {
                        return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
                    }
                    long long idx_val = get<long long>(num_idx->value);
                    if (const auto list_temp = dynamic_cast<List*>(current.get())) {
                        current = list_temp->elements.at(idx_val);
                    }
                    else if (dynamic_cast<String*>(current.get())) {
                        return std::make_pair(nullptr, make_shared<IllegalOperationError>(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Can't assign inside string beyond one level", this->context));
                    }
                    else {
                        return std::make_pair(nullptr, make_shared<IllegalOperationError>(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Can't index a data type which is not iterable", this->context));
                    }
                }
                else {
                    return std::make_pair(nullptr, make_shared<IllegalOperationError>(idx->pos_start.value_or(Position()), idx->pos_end.value_or(Position()), "Invalid Index Type", this->context));
                }
            }

            shared_ptr<DataType> updated_current;
            shared_ptr<RunTimeError> leaf_error;

            if (current->is_dict()) {
                tie(updated_current, leaf_error) = current->assignIndex({ indexes.back() }, val);
            }
            else if (dynamic_cast<List*>(current.get())) {
                auto list_current = dynamic_pointer_cast<List>(current);
                updated_current = dynamic_pointer_cast<DataType>(list_current->copy());

                auto last_idx = indexes.back();
                auto num_last_idx = dynamic_cast<const Number*>(last_idx.get());
                if (!num_last_idx || num_last_idx->is_float) {
                    return std::make_pair(nullptr, make_shared<IllegalOperationError>(last_idx->pos_start.value_or(Position()), last_idx->pos_end.value_or(Position()), "Invalid Index Type", this->context));
                }
                if (holds_alternative<double>(num_last_idx->value)) {
                    return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(last_idx->pos_start.value_or(Position()), last_idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
                }

                try {
                    dynamic_pointer_cast<List>(updated_current)->elements.at(get<long long>(num_last_idx->value)) = val;
                }
                catch (const out_of_range&) {
                    return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(last_idx->pos_start.value_or(Position()), last_idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
                }
            }
            else if (dynamic_cast<String*>(current.get())) {
                tie(updated_current, leaf_error) = current->assignIndex({ indexes.back() }, val);
            }
            else {
                auto bad_idx = indexes.back();
                return std::make_pair(nullptr, make_shared<IllegalOperationError>(bad_idx->pos_start.value_or(Position()), bad_idx->pos_end.value_or(Position()), "Can't index a data type which is not iterable", this->context));
            }

            if (leaf_error) return std::make_pair(nullptr, leaf_error);

            shared_ptr<DataType> rebuilt = updated_current;
            for (size_t i = parent_chain.size(); i-- > 0;) {
                auto [updated_parent, error] = parent_chain[i]->assignIndex({ parent_indexes[i] }, rebuilt);
                if (error) return std::make_pair(nullptr, error);
                rebuilt = updated_parent;
            }

            return std::make_pair(rebuilt, nullptr);
        }
        catch (const out_of_range&) {
            auto bad_idx = indexes.back();
            return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(bad_idx->pos_start.value_or(Position()), bad_idx->pos_end.value_or(Position()), "Index out of bounds", this->context));
        }
    }

    [[nodiscard]] OperationResult add(const shared_ptr<DataType>& operand) const override {
        auto new_list = make_shared<List>(this->elements);
        if (dynamic_cast<const Number*>(operand.get()) || dynamic_cast<const String*>(operand.get()) || operand->is_dict()) {
            if (new_list->elements.size() >= 1000000) {
                return std::make_pair(nullptr, make_shared<ValueError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "List size limit exceeded (max 1,000,000 elements)", this->context));
            }
            new_list->elements.push_back(operand);
            new_list->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(new_list), nullptr);
        }
        if (const auto other_list = dynamic_cast<const List*>(operand.get())) {
            if (new_list->elements.size() + other_list->elements.size() > 1000000) {
                return std::make_pair(nullptr, make_shared<ValueError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "List size limit exceeded (max 1,000,000 elements)", this->context));
            }
            new_list->elements.insert(
                new_list->elements.end(),
                other_list->elements.begin(),
                other_list->elements.end()
            );
            new_list->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(new_list), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Cannot add '" + operand->get_type_name() + "' to a List", context));
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
                    return std::make_pair(std::static_pointer_cast<DataType>(new_list), nullptr);
                }
                catch (...) {
                    return std::make_pair(nullptr, make_shared<IndexOutOfBoundsError>(num->pos_start.value_or(Position()), num->pos_end.value_or(Position()), "Index out of bounds", this->context));
                }
            }
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Index must be of an integer Number type", this->context));
    }

    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType>& operand) const override {
        if (const auto num = dynamic_cast<const Number*>(operand.get())) {
            if (const auto int_val = std::get_if<long long>(&num->value)) {
                if (*int_val < 0) {
                    return std::make_pair(nullptr, make_shared<IllegalOperationError>(num->pos_start.value_or(Position()), num->pos_end.value_or(Position()), "List repetition cannot be negative", this->context));
                }
                if (*int_val > 0 && this->elements.size() > 1000000 / *int_val) {
                    return std::make_pair(nullptr, make_shared<ValueError>(num->pos_start.value_or(Position()), num->pos_end.value_or(Position()), "List size limit exceeded (max 1,000,000 elements)", this->context));
                }
                vector<shared_ptr<DataType>> new_elements;
                new_elements.reserve(this->elements.size() * (*int_val));
                for (long long i = 0; i < *int_val; ++i) {
                    new_elements.insert(new_elements.end(), this->elements.begin(), this->elements.end());
                }
                auto new_list = make_shared<List>(new_elements);
                new_list->set_context(this->context);
                return std::make_pair(std::static_pointer_cast<DataType>(new_list), nullptr);
            }
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected an integer Number type", this->context));
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
            }
            else if (!dynamic_cast<List*>(el.get())) {
                auto [is_eq, error] = el->get_comparison_eq(other);
                if (is_eq && is_eq->is_truthy()) {
                    new_list->elements.erase(new_list->elements.begin() + i);
                    break;
                }
            }
        }
        return std::make_pair(std::static_pointer_cast<DataType>(new_list), nullptr);
    }

    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '%' to a List", context)); }
    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '//' to a List", context)); }
    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '**' to a List", context)); }

    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType>& other) const override {
        if (const auto o = dynamic_cast<const List*>(other.get())) {
            auto new_list_copy = dynamic_pointer_cast<List>(this->copy());
            if (new_list_copy->elements.size() != o->elements.size()) {
                auto res = Number::make_bool(false);
                res->set_context(this->context);
                return std::make_pair(std::static_pointer_cast<DataType>(res), nullptr);
            }

            bool all_match = true;
            for (size_t i = 0; i < new_list_copy->elements.size(); ++i) {
                auto left_copied = new_list_copy->elements[i]->copy();
                auto [is_eq, error] = left_copied->get_comparison_eq(o->elements[i]);
                if (error || !is_eq || !is_eq->is_truthy()) {
                    all_match = false;
                    break;
                }
            }

            auto res = Number::make_bool(all_match);
            res->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(res), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(other->pos_start.value_or(Position()), other->pos_end.value_or(Position()), "Expected a List", context));
    }

    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType>& other) const override {
        auto [eq_res, error] = get_comparison_eq(other);
        if (error) return std::make_pair(nullptr, error);

        auto res = Number::make_bool(!eq_res->is_truthy());
        res->set_context(this->context);
        return std::make_pair(std::static_pointer_cast<DataType>(res), nullptr);
    }

    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '<' to a List", context)); }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '>' to a List", context)); }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '<=' to a List", context)); }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '>=' to a List", context)); }
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply 'and' to a List", context)); }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType>& other) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply 'or' to a List", context)); }
    [[nodiscard]] OperationResult not_by() const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply 'not' to a List", context)); }

    [[nodiscard]] OperationResult bitwise_and(const shared_ptr<DataType>& operand) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '&' to a List", this->context)); }
    [[nodiscard]] OperationResult bitwise_xor(const shared_ptr<DataType>& operand) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '^' to a List", this->context)); }
    [[nodiscard]] OperationResult bitwise_or(const shared_ptr<DataType>& operand) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '|' to a List", this->context)); }
    [[nodiscard]] OperationResult bitwise_not() const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '~' to a List", this->context)); }
    [[nodiscard]] OperationResult lshift(const shared_ptr<DataType>& operand) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '<<' to a List", this->context)); }
    [[nodiscard]] OperationResult rshift(const shared_ptr<DataType>& operand) const override { return std::make_pair(nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position()), pos_end.value_or(Position()), "Cannot apply '>>' to a List", this->context)); }
};