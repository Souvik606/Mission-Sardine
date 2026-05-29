#pragma once
#include <bits/stdc++.h>
#include "data_type.h"
#include "number_type.h"
#include "string_type.h"
#include "list_type.h"
#include "../language_core/error.h"

using namespace std;

class Dict final : public DataType {
private:
    static string get_dict_key(const shared_ptr<DataType>& key) {
        if (const auto n = dynamic_cast<const Number*>(key.get())) {
            if (holds_alternative<long long>(n->value)) return "I:" + std::to_string(std::get<long long>(n->value));
            return "D:" + std::to_string(std::get<double>(n->value));
        }
        else if (const auto s = dynamic_cast<const String*>(key.get())) {
            return "S:" + s->value;
        }
        return "";
    }

public:
    unordered_map<string, shared_ptr<DataType>> elements;
    vector<string> keys_order;

    [[nodiscard]] bool is_dict() const override {
        return true;
    }

    Dict() = default;

    explicit Dict(const vector<pair<shared_ptr<DataType>, shared_ptr<DataType>>>& elements_vec) {
        for (const auto& pair : elements_vec) {
            string key = get_dict_key(pair.first);
            if (!key.empty()) {
                if (elements.find(key) == elements.end()) {
                    keys_order.push_back(key);
                }
                elements[key] = pair.second;
            }
        }
    }

    [[nodiscard]] shared_ptr<DataType> copy() const override {
        auto new_dict = make_shared<Dict>();
        new_dict->set_pos(pos_start, pos_end);
        new_dict->set_context(context);
        new_dict->keys_order = this->keys_order;
        for (const auto& key : keys_order) {
            new_dict->elements[key] = elements.at(key);
        }
        return new_dict;
    }

    [[nodiscard]] string to_string() const override {
        stringstream ss;
        ss << "{";
        bool first = true;
        for (const auto& key : keys_order) {
            if (!first) ss << ", ";
            first = false;
            if (key.substr(0, 2) == "I:") ss << key.substr(2);
            else if (key.substr(0, 2) == "D:") ss << key.substr(2);
            else if (key.substr(0, 2) == "S:") ss << "'" << key.substr(2) << "'";

            ss << ": " << elements.at(key)->to_string();
        }
        ss << "}";
        return ss.str();
    }

    [[nodiscard]] bool is_truthy() const override {
        return elements.size() > 0;
    }

    [[nodiscard]] OperationResult is_true() const override {
        return { make_shared<Number>(static_cast<long long>(elements.size())), nullptr };
    }

    OperationResult add(const shared_ptr<DataType>& operand) const override {
        if (const auto other_dict = dynamic_cast<const Dict*>(operand.get())) {
            auto new_dict = dynamic_pointer_cast<Dict>(this->copy());
            for (const auto& key : other_dict->keys_order) {
                if (new_dict->elements.find(key) == new_dict->elements.end()) {
                    new_dict->keys_order.push_back(key);
                }
                new_dict->elements[key] = other_dict->elements.at(key);
            }
            return { new_dict, nullptr };
        }
        return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Can only add dictionary to another dictionary", context) };
    }

    OperationResult subtract(const shared_ptr<DataType>& operand) const override {
        if (dynamic_cast<const Number*>(operand.get()) || dynamic_cast<const String*>(operand.get())) {
            auto new_dict = dynamic_pointer_cast<Dict>(this->copy());
            string key = get_dict_key(operand);
            if (new_dict->elements.find(key) != new_dict->elements.end()) {
                new_dict->elements.erase(key);
                new_dict->keys_order.erase(remove(new_dict->keys_order.begin(), new_dict->keys_order.end(), key), new_dict->keys_order.end());
                return { new_dict, nullptr };
            }
            return { new_dict, nullptr };
        }
        return { nullptr, make_shared<DictKeyError>(operand->pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Dictionary key must be a number or string", context) };
    }

    OperationResult get_comparison_eq(const shared_ptr<DataType>& operand) const override {
        if (const auto other_dict = dynamic_cast<const Dict*>(operand.get())) {
            if (elements.size() != other_dict->elements.size()) {
                return { make_shared<Number>(0LL), nullptr };
            }
            for (const auto& pair : elements) {
                if (other_dict->elements.find(pair.first) == other_dict->elements.end()) {
                    return { make_shared<Number>(0LL), nullptr };
                }
                const auto& other_value = other_dict->elements.at(pair.first);
                if (const auto left_string = dynamic_cast<const String*>(pair.second.get())) {
                    const auto right_string = dynamic_cast<const String*>(other_value.get());
                    if (!right_string || left_string->value != right_string->value) {
                        return { make_shared<Number>(0LL), nullptr };
                    }
                }
                else if (pair.second->to_string() != other_value->to_string()) {
                    return { make_shared<Number>(0LL), nullptr };
                }
            }
            return { make_shared<Number>(1LL), nullptr };
        }
        return { nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Expected a Dictionary", context) };
    }

    OperationResult get_comparison_neq(const shared_ptr<DataType>& operand) const override {
        auto eq_res = get_comparison_eq(operand);
        if (eq_res.second) {
            return eq_res;
        }
        if (auto t_num = dynamic_cast<Number*>(eq_res.first.get())) {
            return { make_shared<Number>(t_num->is_truthy() ? 0LL : 1LL), nullptr };
        }
        return eq_res;
    }

    OperationResult not_by() const override {
        return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), pos_end.value_or(Position{}), "Cannot apply 'not' to a Dictionary", context) };
    }

    virtual OperationResult multiply(const shared_ptr<DataType>& operand) const override {
        return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply '*' to a Dictionary", context) };
    }

    virtual OperationResult divide(const shared_ptr<DataType>& operand) const override {
        return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply '/' to a Dictionary", context) };
    }

    virtual OperationResult modulus(const shared_ptr<DataType>& operand) const override {
        return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply '%' to a Dictionary", context) };
    }

    virtual OperationResult exponent(const shared_ptr<DataType>& operand) const override {
        return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply '^' to a Dictionary", context) };
    }

    virtual OperationResult floor_divide(const shared_ptr<DataType>& operand) const override {
        return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply '//' to a Dictionary", context) };
    }

    virtual OperationResult get_comparison_lt(const shared_ptr<DataType>& operand) const override {
        return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply '<' to a Dictionary", context) };
    }

    virtual OperationResult get_comparison_gt(const shared_ptr<DataType>& operand) const override {
        return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply '>' to a Dictionary", context) };
    }

    virtual OperationResult get_comparison_lte(const shared_ptr<DataType>& operand) const override {
        return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply '<=' to a Dictionary", context) };
    }

    virtual OperationResult get_comparison_gte(const shared_ptr<DataType>& operand) const override {
        return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply '>=' to a Dictionary", context) };
    }

    virtual OperationResult and_by(const shared_ptr<DataType>& operand) const override {
        return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply 'and' to a Dictionary", context) };
    }

    virtual OperationResult or_by(const shared_ptr<DataType>& operand) const override {
        return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply 'or' to a Dictionary", context) };
    }

    OperationResult getByIndex(const vector<shared_ptr<DataType>>& indexes) const override {
        auto temp = copy();
        try {
            for (const auto& idx : indexes) {
                if (const auto dict_temp = dynamic_cast<Dict*>(temp.get())) {
                    if (dynamic_cast<const Number*>(idx.get()) || dynamic_cast<const String*>(idx.get())) {
                        string key = get_dict_key(idx);
                        if (dict_temp->elements.find(key) != dict_temp->elements.end()) {
                            temp = dict_temp->elements[key];
                        }
                        else {
                            return { nullptr, make_shared<DictKeyError>(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Key does not exist", context) };
                        }
                    }
                    else {
                        return { nullptr, make_shared<DictKeyError>(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Dictionary keys must be numbers or strings", context) };
                    }
                }
                else {
                    auto num_idx = dynamic_cast<const Number*>(idx.get());
                    if (num_idx && holds_alternative<long long>(num_idx->value)) {
                        if (const auto list_temp = dynamic_cast<List*>(temp.get())) {
                            temp = list_temp->elements.at(get<long long>(num_idx->value));
                        }
                        else if (const auto str_temp = dynamic_cast<String*>(temp.get())) {
                            temp = make_shared<String>(string(1, str_temp->value.at(get<long long>(num_idx->value))));
                            temp->set_context(context);
                        }
                        else {
                            return { nullptr, make_shared<RunTimeError>(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Can't index a data type which is not iterable", context, "RunTimeError") };
                        }
                    }
                    else {
                        return { nullptr, make_shared<RunTimeError>(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Invalid Index Type", context) };
                    }
                }
            }
            return { temp, nullptr };
        }
        catch (const out_of_range&) {
            auto bad_idx = indexes.back();
            return { nullptr, make_shared<RunTimeError>(bad_idx->pos_start.value_or(Position{}), bad_idx->pos_end.value_or(Position{}), "Index out of bounds", context, "RunTimeError") };
        }
    }

    OperationResult assignIndex(const vector<shared_ptr<DataType>>& indexes, const shared_ptr<DataType>& val) const override {
        if (indexes.empty()) return { nullptr, make_shared<IndexOutOfBoundsError>(pos_start.value_or(Position{}), pos_end.value_or(Position{}), "Index out of bounds", context) };

        auto new_dict = dynamic_pointer_cast<Dict>(copy());
        shared_ptr<DataType> current = new_dict;

        try {
            vector<shared_ptr<DataType>> parent_chain;
            vector<shared_ptr<DataType>> parent_indexes;

            for (size_t i = 0; i + 1 < indexes.size(); ++i) {
                parent_chain.push_back(current);
                parent_indexes.push_back(indexes[i]);

                const auto& idx = indexes[i];
                if (const auto dict_temp = dynamic_cast<Dict*>(current.get())) {
                    if (dynamic_cast<const Number*>(idx.get()) || dynamic_cast<const String*>(idx.get())) {
                        string key = get_dict_key(idx);
                        if (dict_temp->elements.find(key) != dict_temp->elements.end()) {
                            current = dict_temp->elements[key];
                        }
                        else {
                            return { nullptr, make_shared<DictKeyError>(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Key does not exist", context) };
                        }
                    }
                    else {
                        return { nullptr, make_shared<DictKeyError>(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Dictionary keys must be numbers or strings", context) };
                    }
                }
                else {
                    auto num_idx = dynamic_cast<const Number*>(idx.get());
                    if (num_idx && holds_alternative<long long>(num_idx->value)) {
                        if (const auto list_temp = dynamic_cast<List*>(current.get())) {
                            current = list_temp->elements.at(get<long long>(num_idx->value));
                        }
                        else if (dynamic_cast<String*>(current.get())) {
                            return { nullptr, make_shared<RunTimeError>(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Can't assign inside string beyond one level", context, "RunTimeError") };
                        }
                        else {
                            return { nullptr, make_shared<RunTimeError>(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Can't index a data type which is not iterable", context, "RunTimeError") };
                        }
                    }
                    else {
                        return { nullptr, make_shared<RunTimeError>(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Invalid Index Type", context) };
                    }
                }
            }

            shared_ptr<DataType> updated_current;
            shared_ptr<RunTimeError> leaf_error;
            auto last_idx = indexes.back();

            if (const auto dict_temp = dynamic_cast<Dict*>(current.get())) {
                updated_current = dict_temp->copy();
                auto updated_dict_ptr = dynamic_pointer_cast<Dict>(updated_current);
                if (dynamic_cast<const Number*>(last_idx.get()) || dynamic_cast<const String*>(last_idx.get())) {
                    string key = get_dict_key(last_idx);
                    if (updated_dict_ptr->elements.find(key) == updated_dict_ptr->elements.end()) {
                        updated_dict_ptr->keys_order.push_back(key);
                    }
                    updated_dict_ptr->elements[key] = val;
                }
                else {
                    return { nullptr, make_shared<DictKeyError>(last_idx->pos_start.value_or(Position{}), last_idx->pos_end.value_or(Position{}), "Dictionary keys must be numbers or strings", context) };
                }
            }
            else if (dynamic_cast<List*>(current.get())) {
                tie(updated_current, leaf_error) = current->assignIndex({ last_idx }, val);
            }
            else if (dynamic_cast<String*>(current.get())) {
                tie(updated_current, leaf_error) = current->assignIndex({ last_idx }, val);
            }
            else {
                return { nullptr, make_shared<RunTimeError>(last_idx->pos_start.value_or(Position{}), last_idx->pos_end.value_or(Position{}), "Can't index a data type which is not iterable", context, "RunTimeError") };
            }

            if (leaf_error) return { nullptr, leaf_error };

            shared_ptr<DataType> rebuilt = updated_current;
            for (size_t i = parent_chain.size(); i-- > 0;) {
                auto [updated_parent, error] = parent_chain[i]->assignIndex({ parent_indexes[i] }, rebuilt);
                if (error) return { nullptr, error };
                rebuilt = updated_parent;
            }

            return { rebuilt, nullptr };
        }
        catch (const out_of_range&) {
            auto bad_idx = indexes.back();
            return { nullptr, make_shared<RunTimeError>(bad_idx->pos_start.value_or(Position{}), bad_idx->pos_end.value_or(Position{}), "Index out of bounds", context, "RunTimeError") };
        }
    }

    virtual OperationResult bitwise_and(const shared_ptr<DataType>& operand) const override { return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply '&' to a Dictionary", context) }; }
    virtual OperationResult bitwise_xor(const shared_ptr<DataType>& operand) const override { return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply '^' to a Dictionary", context) }; }
    virtual OperationResult bitwise_or(const shared_ptr<DataType>& operand) const override { return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply '|' to a Dictionary", context) }; }
    virtual OperationResult bitwise_not() const override { return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), pos_end.value_or(Position{}), "Cannot apply '~' to a Dictionary", context) }; }
    virtual OperationResult lshift(const shared_ptr<DataType>& operand) const override { return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply '<<' to a Dictionary", context) }; }
    virtual OperationResult rshift(const shared_ptr<DataType>& operand) const override { return { nullptr, make_shared<IllegalOperationError>(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Cannot apply '>>' to a Dictionary", context) }; }
};