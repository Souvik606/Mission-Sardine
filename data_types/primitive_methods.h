#pragma once
#include <bits/stdc++.h>
#include "data_type.h"
#include "number_type.h"
#include "string_type.h"
#include "list_type.h"
#include "dict_type.h"
#include "null_type.h"
#include "../language_core/error.h"

using namespace std;

// Forward declared is_integer helper
extern bool is_integer(const shared_ptr<DataType>& arg, long long& out_val);

// ─────────────────────────────────────────────────────────────────────────────
// String methods implementation
// ─────────────────────────────────────────────────────────────────────────────
inline DataType::OperationResult String::get_attr(const string& attr_name, const shared_ptr<Context>& calling_context) const {
    auto self_ptr = const_cast<String*>(this)->shared_from_this();

    if (attr_name == "split") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto str_self = dynamic_pointer_cast<String>(self);
            if (args.size() != 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "split() takes exactly 1 argument: (delimiter)", context) };
            }
            auto delim = dynamic_pointer_cast<String>(args[0]);
            if (!delim) {
                return { nullptr, make_shared<IllegalOperationError>(args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()), "Delimiter must be a String", context) };
            }
            if (delim->value.empty()) {
                return { nullptr, make_shared<ValueError>(delim->pos_start.value_or(Position()), delim->pos_end.value_or(Position()), "split delimiter cannot be empty", context) };
            }
            vector<shared_ptr<DataType>> parts;
            string s = str_self->value;
            string d = delim->value;
            size_t pos = 0;
            while ((pos = s.find(d)) != string::npos) {
                if (parts.size() >= 1000000) {
                    return { nullptr, make_shared<ValueError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "Split size limit exceeded (max 1,000,000 elements)", context) };
                }
                auto part = make_shared<String>(s.substr(0, pos));
                part->set_context(context);
                parts.push_back(part);
                s.erase(0, pos + d.length());
            }
            if (parts.size() >= 1000000) {
                return { nullptr, make_shared<ValueError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "Split size limit exceeded (max 1,000,000 elements)", context) };
            }
            auto last_part = make_shared<String>(s);
            last_part->set_context(context);
            parts.push_back(last_part);

            auto out = make_shared<List>(parts);
            out->set_context(context);
            return { out, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("split", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "upper") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto str_self = dynamic_pointer_cast<String>(self);
            if (!args.empty() || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "upper() takes no arguments", context) };
            }
            string s = str_self->value;
            transform(s.begin(), s.end(), s.begin(), ::toupper);
            auto out = make_shared<String>(s);
            out->set_context(context);
            return { out, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("upper", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "lower") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto str_self = dynamic_pointer_cast<String>(self);
            if (!args.empty() || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "lower() takes no arguments", context) };
            }
            string s = str_self->value;
            transform(s.begin(), s.end(), s.begin(), ::tolower);
            auto out = make_shared<String>(s);
            out->set_context(context);
            return { out, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("lower", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "trim") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto str_self = dynamic_pointer_cast<String>(self);
            if (!args.empty() || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "trim() takes no arguments", context) };
            }
            string s = str_self->value;
            s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !isspace(ch);
            }));
            s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !isspace(ch);
            }).base(), s.end());
            auto out = make_shared<String>(s);
            out->set_context(context);
            return { out, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("trim", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "starts_with") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto str_self = dynamic_pointer_cast<String>(self);
            if (args.size() != 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "starts_with() takes exactly 1 argument: (prefix)", context) };
            }
            auto prefix = dynamic_pointer_cast<String>(args[0]);
            if (!prefix) {
                return { nullptr, make_shared<IllegalOperationError>(args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()), "Prefix must be a String", context) };
            }
            bool result = (str_self->value.rfind(prefix->value, 0) == 0);
            return { Number::make_bool(result), nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("starts_with", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "ends_with") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto str_self = dynamic_pointer_cast<String>(self);
            if (args.size() != 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "ends_with() takes exactly 1 argument: (suffix)", context) };
            }
            auto suffix = dynamic_pointer_cast<String>(args[0]);
            if (!suffix) {
                return { nullptr, make_shared<IllegalOperationError>(args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()), "Suffix must be a String", context) };
            }
            bool result = false;
            if (str_self->value.length() >= suffix->value.length()) {
                result = (str_self->value.compare(str_self->value.length() - suffix->value.length(), suffix->value.length(), suffix->value) == 0);
            }
            return { Number::make_bool(result), nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("ends_with", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "replace") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto str_self = dynamic_pointer_cast<String>(self);
            if (args.size() != 2 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "replace() takes exactly 2 arguments: (old, new)", context) };
            }
            auto old_str = dynamic_pointer_cast<String>(args[0]);
            auto new_str = dynamic_pointer_cast<String>(args[1]);
            if (!old_str || !new_str) {
                return { nullptr, make_shared<IllegalOperationError>(args[0]->pos_start.value_or(Position()), args[1]->pos_end.value_or(Position()), "Both old and new arguments must be Strings", context) };
            }
            string s = str_self->value;
            string from = old_str->value;
            string to = new_str->value;
            if (from.empty()) {
                return { nullptr, make_shared<ValueError>(args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()), "Replace target string cannot be empty", context) };
            }
            size_t start_pos = 0;
            while((start_pos = s.find(from, start_pos)) != string::npos) {
                if (s.length() - from.length() + to.length() > 1000000) {
                    return { nullptr, make_shared<ValueError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "String size limit exceeded (max 1,000,000 characters)", context) };
                }
                s.replace(start_pos, from.length(), to);
                start_pos += to.length();
            }
            auto out = make_shared<String>(s);
            out->set_context(context);
            return { out, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("replace", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "find") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto str_self = dynamic_pointer_cast<String>(self);
            if (args.size() != 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "find() takes exactly 1 argument", context) };
            }
            auto sub = dynamic_pointer_cast<String>(args[0]);
            if (!sub) {
                return { nullptr, make_shared<IllegalOperationError>(args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()), "Search term must be a String", context) };
            }
            size_t pos = str_self->value.find(sub->value);
            long long index = (pos == string::npos) ? -1 : static_cast<long long>(pos);
            return { make_shared<Number>(index), nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("find", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "contains") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto str_self = dynamic_pointer_cast<String>(self);
            if (args.size() != 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "contains() takes exactly 1 argument", context) };
            }
            auto sub = dynamic_pointer_cast<String>(args[0]);
            if (!sub) {
                return { nullptr, make_shared<IllegalOperationError>(args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()), "Search term must be a String", context) };
            }
            bool result = (str_self->value.find(sub->value) != string::npos);
            return { Number::make_bool(result), nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("contains", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "is_digit") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto str_self = dynamic_pointer_cast<String>(self);
            if (!args.empty() || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "is_digit() takes no arguments", context) };
            }
            bool all_digits = !str_self->value.empty() && all_of(str_self->value.begin(), str_self->value.end(), ::isdigit);
            return { Number::make_bool(all_digits), nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("is_digit", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "is_alpha") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto str_self = dynamic_pointer_cast<String>(self);
            if (!args.empty() || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "is_alpha() takes no arguments", context) };
            }
            bool all_alpha = !str_self->value.empty() && all_of(str_self->value.begin(), str_self->value.end(), ::isalpha);
            return { Number::make_bool(all_alpha), nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("is_alpha", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    return { nullptr, make_shared<AttributeError>(
        pos_start.value_or(Position()), pos_end.value_or(Position()),
        "'" + get_type_name() + "' object has no attribute '" + attr_name + "'",
        calling_context) };
}

// ─────────────────────────────────────────────────────────────────────────────
// List methods implementation
// ─────────────────────────────────────────────────────────────────────────────
inline DataType::OperationResult List::get_attr(const string& attr_name, const shared_ptr<Context>& calling_context) const {
    auto self_ptr = const_cast<List*>(this)->shared_from_this();

    if (attr_name == "append") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto list_self = dynamic_pointer_cast<List>(self);
            if (args.size() != 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "append() takes exactly 1 argument", context) };
            }
            if (list_self->elements.size() >= 1000000) {
                return { nullptr, make_shared<ValueError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "List size limit exceeded (max 1,000,000 elements)", context) };
            }
            list_self->elements.push_back(args[0]);
            return { list_self, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("append", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "prepend") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto list_self = dynamic_pointer_cast<List>(self);
            if (args.size() != 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "prepend() takes exactly 1 argument", context) };
            }
            if (list_self->elements.size() >= 1000000) {
                return { nullptr, make_shared<ValueError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "List size limit exceeded (max 1,000,000 elements)", context) };
            }
            list_self->elements.insert(list_self->elements.begin(), args[0]);
            return { list_self, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("prepend", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "insert") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto list_self = dynamic_pointer_cast<List>(self);
            if (args.size() != 2 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "insert() takes exactly 2 arguments: (index, item)", context) };
            }
            if (list_self->elements.size() >= 1000000) {
                return { nullptr, make_shared<ValueError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "List size limit exceeded (max 1,000,000 elements)", context) };
            }
            auto num_idx = dynamic_pointer_cast<Number>(args[0]);
            if (!num_idx || num_idx->is_float) {
                return { nullptr, make_shared<IllegalOperationError>(args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()), "Index must be an integer Number", context) };
            }
            if (holds_alternative<double>(num_idx->value)) {
                return { nullptr, make_shared<IndexOutOfBoundsError>(args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()), "Index too large (overflow)", context) };
            }
            long long idx_val = get<long long>(num_idx->value);
            if (idx_val < 0) {
                idx_val = static_cast<long long>(list_self->elements.size()) + idx_val;
            }
            idx_val = max(0LL, min(idx_val, static_cast<long long>(list_self->elements.size())));
            list_self->elements.insert(list_self->elements.begin() + idx_val, args[1]);
            return { list_self, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("insert", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "pop") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto list_self = dynamic_pointer_cast<List>(self);
            if (args.size() > 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "pop() takes at most 1 argument: [index]", context) };
            }
            long long idx_val = -1;
            if (args.size() == 1) {
                auto num_idx = dynamic_pointer_cast<Number>(args[0]);
                if (!num_idx || num_idx->is_float) {
                    return { nullptr, make_shared<IllegalOperationError>(args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()), "Index must be an integer Number", context) };
                }
                if (holds_alternative<double>(num_idx->value)) {
                    return { nullptr, make_shared<IndexOutOfBoundsError>(args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()), "Index too large (overflow)", context) };
                }
                idx_val = get<long long>(num_idx->value);
            }
            long long actual_idx = idx_val;
            if (actual_idx < 0) {
                actual_idx = static_cast<long long>(list_self->elements.size()) + actual_idx;
            }
            if (actual_idx < 0 || actual_idx >= static_cast<long long>(list_self->elements.size())) {
                return { nullptr, make_shared<IndexOutOfBoundsError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "Index out of bounds", context) };
            }
            auto popped = list_self->elements[actual_idx];
            list_self->elements.erase(list_self->elements.begin() + actual_idx);
            return { popped, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("pop", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "remove") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto list_self = dynamic_pointer_cast<List>(self);
            if (args.size() != 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "remove() takes exactly 1 argument", context) };
            }
            auto target = args[0];
            int found_idx = -1;
            for (size_t i = 0; i < list_self->elements.size(); ++i) {
                auto [eq_node, err] = list_self->elements[i]->get_comparison_eq(target);
                if (eq_node && eq_node->is_truthy()) {
                    found_idx = static_cast<int>(i);
                    break;
                }
            }
            if (found_idx == -1) {
                return { nullptr, make_shared<IllegalOperationError>(target->pos_start.value_or(Position()), target->pos_end.value_or(Position()), "Element not found in list", context) };
            }
            list_self->elements.erase(list_self->elements.begin() + found_idx);
            return { list_self, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("remove", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "clear") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto list_self = dynamic_pointer_cast<List>(self);
            if (!args.empty() || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "clear() takes no arguments", context) };
            }
            list_self->elements.clear();
            return { list_self, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("clear", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "sort") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto list_self = dynamic_pointer_cast<List>(self);
            bool descending = false;
            if (args.size() > 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "sort() takes at most 1 argument: [descending]", context) };
            }
            if (args.size() == 1) {
                long long desc_val = 0;
                if (!is_integer(args[0], desc_val)) {
                    return { nullptr, make_shared<IllegalOperationError>(args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()), "descending argument must be a Boolean Number (0 or 1)", context) };
                }
                descending = (desc_val != 0);
            }
            vector<shared_ptr<DataType>> sorted_elements = list_self->elements;
            bool sort_error = false;
            shared_ptr<RunTimeError> err_obj;
            sort(sorted_elements.begin(), sorted_elements.end(), [&](const shared_ptr<DataType>& a, const shared_ptr<DataType>& b) {
                if (sort_error) return false;
                auto [lt_res, err] = descending ? b->get_comparison_lt(a) : a->get_comparison_lt(b);
                if (err) {
                    sort_error = true;
                    err_obj = err;
                    return false;
                }
                return lt_res && lt_res->is_truthy();
            });
            if (sort_error) {
                return { nullptr, make_shared<IllegalOperationError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "List elements are not comparable for sorting", context) };
            }
            auto out = make_shared<List>(sorted_elements);
            out->set_context(self->context);
            return { out, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("sort", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "reverse") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto list_self = dynamic_pointer_cast<List>(self);
            if (!args.empty() || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "reverse() takes no arguments", context) };
            }
            vector<shared_ptr<DataType>> rev_elements = list_self->elements;
            std::reverse(rev_elements.begin(), rev_elements.end());
            auto out = make_shared<List>(rev_elements);
            out->set_context(self->context);
            return { out, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("reverse", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "slice") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto list_self = dynamic_pointer_cast<List>(self);
            if (args.size() != 2 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "slice() takes exactly 2 arguments: (start, end)", context) };
            }
            long long start_val = 0;
            long long end_val = 0;
            long long n = static_cast<long long>(list_self->elements.size());
            auto get_bound = [&](const shared_ptr<DataType>& arg, long long& out_val) -> bool {
                auto num = dynamic_pointer_cast<Number>(arg);
                if (!num || num->is_float) return false;
                if (holds_alternative<long long>(num->value)) {
                    out_val = get<long long>(num->value);
                    if (out_val < 0) out_val = n + out_val;
                    out_val = max(0LL, min(out_val, n));
                } else {
                    double val = get<double>(num->value);
                    out_val = (val >= 0.0) ? n : 0LL;
                }
                return true;
            };
            if (!get_bound(args[0], start_val) || !get_bound(args[1], end_val)) {
                return { nullptr, make_shared<IllegalOperationError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "Slice bounds must be integer Numbers", context) };
            }
            vector<shared_ptr<DataType>> sliced;
            for (long long i = start_val; i < end_val; ++i) {
                sliced.push_back(list_self->elements[i]->copy());
            }
            auto out = make_shared<List>(sliced);
            out->set_context(self->context);
            return { out, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("slice", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "join") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto list_self = dynamic_pointer_cast<List>(self);
            if (args.size() != 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "join() takes exactly 1 argument: (separator)", context) };
            }
            auto sep = dynamic_pointer_cast<String>(args[0]);
            if (!sep) {
                return { nullptr, make_shared<IllegalOperationError>(args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()), "Separator must be a String", context) };
            }
            string result = "";
            for (size_t i = 0; i < list_self->elements.size(); ++i) {
                auto el = list_self->elements[i];
                if (auto str_el = dynamic_pointer_cast<String>(el)) {
                    result += str_el->value;
                } else {
                    result += el->to_string();
                }
                if (i + 1 < list_self->elements.size()) {
                    result += sep->value;
                }
            }
            auto out = make_shared<String>(result);
            out->set_context(self->context);
            return { out, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("join", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "index_of") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto list_self = dynamic_pointer_cast<List>(self);
            if (args.size() != 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "index_of() takes exactly 1 argument", context) };
            }
            auto target = args[0];
            long long index = -1;
            for (size_t i = 0; i < list_self->elements.size(); ++i) {
                auto [eq_node, err] = list_self->elements[i]->get_comparison_eq(target);
                if (eq_node && eq_node->is_truthy()) {
                    index = static_cast<long long>(i);
                    break;
                }
            }
            return { make_shared<Number>(index), nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("index_of", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "contains") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto list_self = dynamic_pointer_cast<List>(self);
            if (args.size() != 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "contains() takes exactly 1 argument", context) };
            }
            auto target = args[0];
            bool found = false;
            for (size_t i = 0; i < list_self->elements.size(); ++i) {
                auto [eq_node, err] = list_self->elements[i]->get_comparison_eq(target);
                if (eq_node && eq_node->is_truthy()) {
                    found = true;
                    break;
                }
            }
            return { Number::make_bool(found), nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("contains", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "extend") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto list_self = dynamic_pointer_cast<List>(self);
            if (args.size() != 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "extend() takes exactly 1 argument", context) };
            }
            auto other = dynamic_pointer_cast<List>(args[0]);
            if (!other) {
                return { nullptr, make_shared<IllegalOperationError>(args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()), "Argument to extend() must be a List", context) };
            }
            if (list_self->elements.size() + other->elements.size() > 1000000) {
                return { nullptr, make_shared<ValueError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "List size limit exceeded (max 1,000,000 elements)", context) };
            }
            for (const auto& el : other->elements) {
                list_self->elements.push_back(el->copy());
            }
            return { list_self, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("extend", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "copy") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto list_self = dynamic_pointer_cast<List>(self);
            if (!args.empty() || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "copy() takes no arguments", context) };
            }
            auto out = list_self->copy();
            out->set_context(self->context);
            return { out, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("copy", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    return { nullptr, make_shared<AttributeError>(
        pos_start.value_or(Position()), pos_end.value_or(Position()),
        "'" + get_type_name() + "' object has no attribute '" + attr_name + "'",
        calling_context) };
}

// ─────────────────────────────────────────────────────────────────────────────
// Dict methods implementation
// ─────────────────────────────────────────────────────────────────────────────
inline DataType::OperationResult Dict::get_attr(const string& attr_name, const shared_ptr<Context>& calling_context) const {
    auto self_ptr = const_cast<Dict*>(this)->shared_from_this();

    auto get_ordered_keys = [](const Dict* d, const shared_ptr<Context>& ctx) -> vector<shared_ptr<DataType>> {
        vector<shared_ptr<DataType>> list_keys;
        for (const auto& key : d->keys_order) {
            shared_ptr<DataType> node;
            if (key.substr(0, 2) == "I:") {
                node = make_shared<Number>(stoll(key.substr(2)));
            } else if (key.substr(0, 2) == "D:") {
                node = make_shared<Number>(stod(key.substr(2)));
            } else {
                node = make_shared<String>(key.substr(2));
            }
            node->set_context(ctx);
            list_keys.push_back(node);
        }
        return list_keys;
    };

    if (attr_name == "keys") {
        auto impl = [get_ordered_keys](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto dict_self = dynamic_pointer_cast<Dict>(self);
            if (!args.empty() || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "keys() takes no arguments", context) };
            }
            auto keys = get_ordered_keys(dict_self.get(), context);
            auto out = make_shared<List>(keys);
            out->set_context(context);
            return { out, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("keys", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "values") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto dict_self = dynamic_pointer_cast<Dict>(self);
            if (!args.empty() || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "values() takes no arguments", context) };
            }
            vector<shared_ptr<DataType>> list_vals;
            for (const auto& key : dict_self->keys_order) {
                list_vals.push_back(dict_self->elements.at(key)->copy());
            }
            auto out = make_shared<List>(list_vals);
            out->set_context(context);
            return { out, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("values", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "entries" || attr_name == "items") {
        auto impl = [get_ordered_keys](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto dict_self = dynamic_pointer_cast<Dict>(self);
            if (!args.empty() || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "entries() takes no arguments", context) };
            }
            auto keys = get_ordered_keys(dict_self.get(), context);
            vector<shared_ptr<DataType>> pairs;
            for (size_t i = 0; i < keys.size(); ++i) {
                auto k_node = keys[i];
                auto v_node = dict_self->elements.at(dict_self->keys_order[i])->copy();
                auto pair_list = make_shared<List>(vector<shared_ptr<DataType>>{ k_node, v_node });
                pair_list->set_context(context);
                pairs.push_back(pair_list);
            }
            auto out = make_shared<List>(pairs);
            out->set_context(context);
            return { out, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>(attr_name, self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "get") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto dict_self = dynamic_pointer_cast<Dict>(self);
            if (args.empty() || args.size() > 2 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "get() takes 1 or 2 arguments: (key, [default])", context) };
            }
            auto key = args[0];
            if (!dynamic_pointer_cast<Number>(key) && !dynamic_pointer_cast<String>(key)) {
                return { nullptr, make_shared<IllegalOperationError>(key->pos_start.value_or(Position()), key->pos_end.value_or(Position()), "Key must be a Number or String", context) };
            }
            string key_str = dict_self->get_dict_key(key);
            if (dict_self->elements.find(key_str) == dict_self->elements.end()) {
                if (args.size() == 2) {
                    return { args[1], nullptr };
                }
                return { Number::make(0LL), nullptr };
            }
            return { dict_self->elements.at(key_str)->copy(), nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("get", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "has_key" || attr_name == "contains") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto dict_self = dynamic_pointer_cast<Dict>(self);
            if (args.size() != 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "has_key() takes exactly 1 argument", context) };
            }
            auto key = args[0];
            if (!dynamic_pointer_cast<Number>(key) && !dynamic_pointer_cast<String>(key)) {
                return { nullptr, make_shared<IllegalOperationError>(key->pos_start.value_or(Position()), key->pos_end.value_or(Position()), "Key must be a Number or String", context) };
            }
            string key_str = dict_self->get_dict_key(key);
            bool found = (dict_self->elements.find(key_str) != dict_self->elements.end());
            return { Number::make_bool(found), nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>(attr_name, self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "pop") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto dict_self = dynamic_pointer_cast<Dict>(self);
            if (args.empty() || args.size() > 2 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "pop() takes 1 or 2 arguments: (key, [default])", context) };
            }
            auto key = args[0];
            if (!dynamic_pointer_cast<Number>(key) && !dynamic_pointer_cast<String>(key)) {
                return { nullptr, make_shared<IllegalOperationError>(key->pos_start.value_or(Position()), key->pos_end.value_or(Position()), "Key must be a Number or String", context) };
            }
            string key_str = dict_self->get_dict_key(key);
            if (dict_self->elements.find(key_str) == dict_self->elements.end()) {
                if (args.size() == 2) {
                    return { args[1], nullptr };
                }
                return { nullptr, make_shared<DictKeyError>(key->pos_start.value_or(Position()), key->pos_end.value_or(Position()), "Key not found in dictionary", context) };
            }
            auto popped = dict_self->elements.at(key_str);
            dict_self->elements.erase(key_str);
            dict_self->keys_order.erase(std::remove(dict_self->keys_order.begin(), dict_self->keys_order.end(), key_str), dict_self->keys_order.end());
            return { popped, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("pop", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "pop_item") {
        auto impl = [get_ordered_keys](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto dict_self = dynamic_pointer_cast<Dict>(self);
            if (!args.empty() || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "pop_item() takes no arguments", context) };
            }
            if (dict_self->keys_order.empty()) {
                return { nullptr, make_shared<IllegalOperationError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "pop_item() called on empty dictionary", context) };
            }
            string last_key = dict_self->keys_order.back();
            auto keys = get_ordered_keys(dict_self.get(), context);
            auto k_node = keys.back();
            auto v_node = dict_self->elements.at(last_key);
            dict_self->elements.erase(last_key);
            dict_self->keys_order.pop_back();

            auto pair_list = make_shared<List>(vector<shared_ptr<DataType>>{ k_node, v_node });
            pair_list->set_context(context);
            return { pair_list, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("pop_item", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "update") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto dict_self = dynamic_pointer_cast<Dict>(self);
            if (args.size() != 1 || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "update() takes exactly 1 argument: (other_dict)", context) };
            }
            auto other = dynamic_pointer_cast<Dict>(args[0]);
            if (!other) {
                return { nullptr, make_shared<IllegalOperationError>(args[0]->pos_start.value_or(Position()), args[0]->pos_end.value_or(Position()), "Argument to update() must be a Dictionary", context) };
            }
            size_t combined_keys = dict_self->elements.size();
            for (const auto& key : other->keys_order) {
                if (dict_self->elements.find(key) == dict_self->elements.end()) {
                    combined_keys++;
                }
            }
            if (combined_keys > 100000) {
                return { nullptr, make_shared<ValueError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "Dictionary size limit exceeded (max 100,000 elements)", context) };
            }
            for (const auto& key : other->keys_order) {
                if (dict_self->elements.find(key) == dict_self->elements.end()) {
                    dict_self->keys_order.push_back(key);
                }
                dict_self->elements[key] = other->elements.at(key)->copy();
            }
            return { dict_self, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("update", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "clear") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto dict_self = dynamic_pointer_cast<Dict>(self);
            if (!args.empty() || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "clear() takes no arguments", context) };
            }
            dict_self->elements.clear();
            dict_self->keys_order.clear();
            return { dict_self, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("clear", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    if (attr_name == "copy") {
        auto impl = [](const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) -> OperationResult {
            auto dict_self = dynamic_pointer_cast<Dict>(self);
            if (!args.empty() || !kw_args.empty()) {
                return { nullptr, make_shared<ArgumentError>(self->pos_start.value_or(Position()), self->pos_end.value_or(Position()), "copy() takes no arguments", context) };
            }
            auto out = dict_self->copy();
            out->set_context(self->context);
            return { out, nullptr };
        };
        shared_ptr<DataType> bound = make_shared<BoundMethod>("copy", self_ptr, impl);
        bound->set_context(calling_context).set_pos(pos_start, pos_end);
        return { bound, nullptr };
    }

    return { nullptr, make_shared<AttributeError>(
        pos_start.value_or(Position()), pos_end.value_or(Position()),
        "'" + get_type_name() + "' object has no attribute '" + attr_name + "'",
        calling_context) };
}
