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
        return new_list;
    }

    [[nodiscard]] string to_string() const override {
        stringstream ss;
        ss << "[";
        for (size_t i = 0; i < elements.size(); ++i) {
            if (elements[i]) {
                ss << elements[i]->to_string();
            } else {
                ss << "null";
            }
            if (i < elements.size() - 1) {
                ss << ", ";
            }
        }
        ss << "]";
        return ss.str();
    }

    [[nodiscard]] bool is_truthy() const override {
        return !elements.empty();
    }

    [[nodiscard]] OperationResult add(const shared_ptr<DataType>& operand) const override {
        auto new_list = make_shared<List>(this->elements);
        if (dynamic_cast<const Number*>(operand.get()) || dynamic_cast<const String*>(operand.get())) {
            new_list->elements.push_back(operand);
            new_list->set_context(this->context);
            return {new_list, nullopt};
        }
        if (const auto other_list = dynamic_cast<const List*>(operand.get())) {
            new_list->elements.insert(
                new_list->elements.end(),
                other_list->elements.begin(),
                other_list->elements.end()
            );
            new_list->set_context(this->context);
            return {new_list, nullopt};
        }
        return {nullptr, illegal_op_for_list(this, operand.get())};
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
                    return {new_list, nullopt};
                } catch (...) {
                    return {nullptr, RunTimeError(num->pos_start.value_or(Position()), num->pos_end.value_or(Position()), "Index out of bounds", this->context)};
                }
            }
            return {nullptr, RunTimeError(num->pos_start.value_or(Position()), num->pos_end.value_or(Position()), "Index must be an integer", this->context)};
        }
        return {nullptr, illegal_op_for_list(this, operand.get())};
    }

    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType>& operand) const override {
        if (const auto num = dynamic_cast<const Number*>(operand.get())) {
            if (const auto int_val = std::get_if<long long>(&num->value)) {
                vector<shared_ptr<DataType>> new_elements;
                for (long long i = 0; i < *int_val; ++i) {
                    new_elements.insert(new_elements.end(), this->elements.begin(), this->elements.end());
                }
                auto new_list = make_shared<List>(new_elements);
                new_list->set_context(this->context);
                return {new_list, nullopt};
            }
            return {nullptr, RunTimeError(num->pos_start.value_or(Position()), num->pos_end.value_or(Position()), "List can only be multiplied by an integer", this->context)};
        }
        return {nullptr, illegal_op_for_list(this, operand.get())};
    }

    [[nodiscard]] OperationResult divide(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_list(this, other.get())};
    }
    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_list(this, other.get())};
    }
    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_list(this, other.get())};
    }
    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_list(this, other.get())};
    }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_list(this, other.get())};
    }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_list(this, other.get())};
    }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_list(this, other.get())};
    }
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_list(this, other.get())};
    }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType>& other) const override {
        return {nullptr, illegal_op_for_list(this, other.get())};
    }
    [[nodiscard]] OperationResult not_by() const override {
        return {nullptr, illegal_op_for_list(this)};
    }
};