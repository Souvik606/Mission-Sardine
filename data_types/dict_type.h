#pragma once
#include <bits/stdc++.h>
#include "data_type.h"
#include "number_type.h"
#include "string_type.h"
#include "list_type.h"
#include "../language_core/error.h"

using namespace std;

class Dict;

class Dict final : public DataType {
private:
    static string get_dict_key(const shared_ptr<DataType>& key) {
        if (const auto n = dynamic_cast<const Number*>(key.get())) {
            if (holds_alternative<long long>(n->value)) return "I:" + std::to_string(std::get<long long>(n->value));
            return "D:" + std::to_string(std::get<double>(n->value));
        } else if (const auto s = dynamic_cast<const String*>(key.get())) {
            return "S:" + s->value;
        }
        return "";
    }

public:
    unordered_map<string, shared_ptr<DataType>> elements;

    Dict() = default;

    explicit Dict(const vector<pair<shared_ptr<DataType>, shared_ptr<DataType>>>& elements_vec) {
        for (const auto& pair : elements_vec) {
            string key = get_dict_key(pair.first);
            if (!key.empty()) {
                elements[key] = pair.second;
            }
        }
    }

    [[nodiscard]] shared_ptr<DataType> copy() const override {
        auto new_dict = make_shared<Dict>();
        new_dict->set_pos(pos_start, pos_end);
        new_dict->set_context(context);
        for (const auto& pair : elements) {
            new_dict->elements[pair.first] = pair.second->copy();
        }
        return new_dict;
    }

    [[nodiscard]] string to_string() const override {
        stringstream ss;
        ss << "{";
        bool first = true;
        for (const auto& pair : elements) {
            if (!first) ss << ", ";
            first = false;
            if (pair.first.substr(0, 2) == "I:") ss << pair.first.substr(2);
            else if (pair.first.substr(0, 2) == "D:") ss << pair.first.substr(2);
            else if (pair.first.substr(0, 2) == "S:") ss << "\"" << pair.first.substr(2) << "\"";
            
            ss << ": " << pair.second->to_string();
        }
        ss << "}";
        return ss.str();
    }

    [[nodiscard]] bool is_truthy() const override {
        return elements.size() > 0;
    }

    [[nodiscard]] OperationResult is_true() const override {
        return {make_shared<Number>(static_cast<long long>(elements.size())), nullopt};
    }

    OperationResult add(const shared_ptr<DataType>& operand) const override {
        if (const auto other_dict = dynamic_cast<const Dict*>(operand.get())) {
            auto new_dict = dynamic_pointer_cast<Dict>(this->copy());
            for (const auto& pair : other_dict->elements) {
                new_dict->elements[pair.first] = pair.second->copy();
            }
            return {new_dict, nullopt};
        }
        return {nullptr, IllegalOperationError(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Can only add Dict to Dict", context)};
    }
    
    OperationResult subtract(const shared_ptr<DataType>& operand) const override {
        if (dynamic_cast<const Number*>(operand.get()) || dynamic_cast<const String*>(operand.get())) {
            auto new_dict = dynamic_pointer_cast<Dict>(this->copy());
            string key = get_dict_key(operand);
            if (new_dict->elements.find(key) != new_dict->elements.end()) {
                new_dict->elements.erase(key);
                return {new_dict, nullopt};
            }
            return {nullptr, DictKeyError(operand->pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Key does not exist", context)};
        }
        return {nullptr, IllegalOperationError(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Can only subtract Number or String from Dict", context)};
    }

    OperationResult get_comparison_eq(const shared_ptr<DataType>& operand) const override {
        if (const auto other_dict = dynamic_cast<const Dict*>(operand.get())) {
            if (elements.size() != other_dict->elements.size()) {
                return {make_shared<Number>(0LL), nullopt};
            }
            for (const auto& pair : elements) {
                if (other_dict->elements.find(pair.first) == other_dict->elements.end()) {
                    return {make_shared<Number>(0LL), nullopt};
                }
                auto res = pair.second->get_comparison_eq(other_dict->elements.at(pair.first));
                if (auto t_num = dynamic_cast<Number*>(res.first.get())) {
                     if (t_num->is_truthy() == 0) return {make_shared<Number>(0LL), nullopt};
                }
            }
            return {make_shared<Number>(1LL), nullopt};
        }
        return {make_shared<Number>(0LL), nullopt};
    }

    OperationResult get_comparison_neq(const shared_ptr<DataType>& operand) const override {
        auto eq_res = get_comparison_eq(operand);
        if (auto t_num = dynamic_cast<Number*>(eq_res.first.get())) {
            return {make_shared<Number>(t_num->is_truthy() ? 0LL : 1LL), nullopt};
        }
        return eq_res;
    }

    OperationResult not_by() const override {
        return {nullptr, IllegalOperationError(pos_start.value_or(Position{}), pos_end.value_or(Position{}), "Cannot 'not' a Dict", context)};
    }

    virtual OperationResult multiply(const shared_ptr<DataType>& operand) const override {
        return {nullptr, IllegalOperationError(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Dict does not support multiplication", context)};
    }
    
    virtual OperationResult divide(const shared_ptr<DataType>& operand) const override {
        return {nullptr, IllegalOperationError(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Dict does not support division", context)};
    }

    virtual OperationResult modulus(const shared_ptr<DataType>& operand) const override {
        return {nullptr, IllegalOperationError(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Dict does not support modulus", context)};
    }

    virtual OperationResult exponent(const shared_ptr<DataType>& operand) const override {
        return {nullptr, IllegalOperationError(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Dict does not support exponentiation", context)};
    }

    virtual OperationResult floor_divide(const shared_ptr<DataType>& operand) const override {
        return {nullptr, IllegalOperationError(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Dict does not support floor division", context)};
    }

    virtual OperationResult get_comparison_lt(const shared_ptr<DataType>& operand) const override {
        return {nullptr, IllegalOperationError(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Dict does not support less than", context)};
    }

    virtual OperationResult get_comparison_gt(const shared_ptr<DataType>& operand) const override {
        return {nullptr, IllegalOperationError(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Dict does not support greater than", context)};
    }

    virtual OperationResult get_comparison_lte(const shared_ptr<DataType>& operand) const override {
        return {nullptr, IllegalOperationError(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Dict does not support less than or equal", context)};
    }

    virtual OperationResult get_comparison_gte(const shared_ptr<DataType>& operand) const override {
        return {nullptr, IllegalOperationError(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Dict does not support greater than or equal", context)};
    }

    virtual OperationResult and_by(const shared_ptr<DataType>& operand) const override {
        return {nullptr, IllegalOperationError(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Dict does not support logical AND", context)};
    }

    virtual OperationResult or_by(const shared_ptr<DataType>& operand) const override {
        return {nullptr, IllegalOperationError(pos_start.value_or(Position{}), operand->pos_end.value_or(Position{}), "Dict does not support logical OR", context)};
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
                        } else {
                            return {nullptr, DictKeyError(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Key does not exist", context)};
                        }
                    } else {
                        return {nullptr, DictKeyError(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Dictionary keys must be numbers or strings", context)};
                    }
                } else {
                    auto num_idx = dynamic_cast<const Number*>(idx.get());
                    if (num_idx && holds_alternative<long long>(num_idx->value)) {
                        if (const auto list_temp = dynamic_cast<List*>(temp.get())) {
                            temp = list_temp->elements.at(get<long long>(num_idx->value));
                        } else if (const auto str_temp = dynamic_cast<String*>(temp.get())) {
                            temp = make_shared<String>(string(1, str_temp->value.at(get<long long>(num_idx->value))));
                            temp->set_context(context);
                        } else {
                            return {nullptr, RunTimeError(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Can't index a data type which is not iterable", context, "RunTimeError")};
                        }
                    } else {
                        return {nullptr, RunTimeError(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Invalid Index Type", context, "RunTimeError")};
                    }
                }
            }
            return {temp, nullopt};
        } catch (const out_of_range&) {
            auto bad_idx = indexes.back();
            return {nullptr, RunTimeError(bad_idx->pos_start.value_or(Position{}), bad_idx->pos_end.value_or(Position{}), "Index out of bounds", context, "RunTimeError")};
        }
    }

    OperationResult assignIndex(const vector<shared_ptr<DataType>>& indexes, const shared_ptr<DataType>& val) const override {
        auto new_dict = dynamic_pointer_cast<Dict>(copy());
        shared_ptr<DataType> temp = new_dict;
        
        try {
            for (size_t i = 0; i < indexes.size() - 1; ++i) {
                const auto& idx = indexes[i];
                if (const auto dict_temp = dynamic_cast<Dict*>(temp.get())) {
                    if (dynamic_cast<const Number*>(idx.get()) || dynamic_cast<const String*>(idx.get())) {
                        string key = get_dict_key(idx);
                        if (dict_temp->elements.find(key) != dict_temp->elements.end()) {
                            temp = dict_temp->elements[key];
                        } else {
                            return {nullptr, DictKeyError(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Key does not exist", context)};
                        }
                    } else {
                        return {nullptr, DictKeyError(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Dictionary keys must be numbers or strings", context)};
                    }
                } else {
                    auto num_idx = dynamic_cast<const Number*>(idx.get());
                    if (num_idx && holds_alternative<long long>(num_idx->value)) {
                        if (const auto list_temp = dynamic_cast<List*>(temp.get())) {
                            temp = list_temp->elements.at(get<long long>(num_idx->value));
                        } else if (dynamic_cast<String*>(temp.get())) {
                            return {nullptr, RunTimeError(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Can't assign inside string beyond one level", context, "RunTimeError")};
                        } else {
                            return {nullptr, RunTimeError(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Can't index a data type which is not iterable", context, "RunTimeError")};
                        }
                    } else {
                        return {nullptr, RunTimeError(idx->pos_start.value_or(Position{}), idx->pos_end.value_or(Position{}), "Invalid Index Type", context, "RunTimeError")};
                    }
                }
            }

            auto last_idx = indexes.back();
            
            if (const auto dict_temp = dynamic_cast<Dict*>(temp.get())) {
                if (dynamic_cast<const Number*>(last_idx.get()) || dynamic_cast<const String*>(last_idx.get())) {
                    string key = get_dict_key(last_idx);
                    dict_temp->elements[key] = val;
                    return {new_dict, nullopt};
                }
                return {nullptr, DictKeyError(last_idx->pos_start.value_or(Position{}), last_idx->pos_end.value_or(Position{}), "Dictionary keys must be numbers or strings", context)};
            }

            auto num_last_idx = dynamic_cast<const Number*>(last_idx.get());
            if (!num_last_idx || !holds_alternative<long long>(num_last_idx->value)) {
                return {nullptr, RunTimeError(last_idx->pos_start.value_or(Position{}), last_idx->pos_end.value_or(Position{}), "Invalid Index Type", context, "RunTimeError")};
            }

            if (const auto list_temp = dynamic_cast<List*>(temp.get())) {
                list_temp->elements.at(get<long long>(num_last_idx->value)) = val;
                return {new_dict, nullopt};
            } else if (const auto str_temp = dynamic_cast<String*>(temp.get())) {
                auto val_str = dynamic_cast<const String*>(val.get());
                if (!val_str || val_str->value.length() != 1) {
                    return {nullptr, RunTimeError(val->pos_start.value_or(Position{}), val->pos_end.value_or(Position{}), "Assigned value must be a single character string", context, "RunTimeError")};
                }
                
                string s = str_temp->value;
                s.at(get<long long>(num_last_idx->value)) = val_str->value[0];
                auto replaced = make_shared<String>(s);
                replaced->set_context(context);

                shared_ptr<DataType> parent = new_dict;
                for (size_t i = 0; i < indexes.size() - 2; ++i) {
                    if (auto p_dict = dynamic_cast<Dict*>(parent.get())) {
                        parent = p_dict->elements[get_dict_key(indexes[i])];
                    } else if (auto p_list = dynamic_cast<List*>(parent.get())) {
                         parent = p_list->elements.at(get<long long>(dynamic_cast<const Number*>(indexes[i].get())->value));
                    }
                }
                
                auto target_idx = indexes[indexes.size() - 2];
                if (auto p_dict = dynamic_cast<Dict*>(parent.get())) {
                    p_dict->elements[get_dict_key(target_idx)] = replaced;
                } else if (auto p_list = dynamic_cast<List*>(parent.get())) {
                     p_list->elements.at(get<long long>(dynamic_cast<const Number*>(target_idx.get())->value)) = replaced;
                }
                return {new_dict, nullopt};
            } else {
                return {nullptr, RunTimeError(last_idx->pos_start.value_or(Position{}), last_idx->pos_end.value_or(Position{}), "Can't index a data type which is not iterable", context, "RunTimeError")};
            }
        } catch (const out_of_range&) {
            auto bad_idx = indexes.back();
            return {nullptr, RunTimeError(bad_idx->pos_start.value_or(Position{}), bad_idx->pos_end.value_or(Position{}), "Index out of bounds", context, "RunTimeError")};
        }
    }
};