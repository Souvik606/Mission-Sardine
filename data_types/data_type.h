#pragma once
#include <bits/stdc++.h>
#include "../language_core/position.h"
#include "../language_core/context.h"
#include "../language_core/error.h"

using namespace std;

class DataType;
class Number;
class Context;

class ContextRef {
private:
    std::weak_ptr<Context> ptr;
public:
    ContextRef() = default;
    ContextRef(const std::shared_ptr<Context>& s) : ptr(s) {}
    ContextRef(std::shared_ptr<Context>&& s) : ptr(std::move(s)) {}
    ContextRef(const ContextRef&) = default;
    ContextRef(ContextRef&&) = default;

    ContextRef& operator=(const std::shared_ptr<Context>& s) {
        ptr = s;
        return *this;
    }
    ContextRef& operator=(std::shared_ptr<Context>&& s) {
        ptr = std::move(s);
        return *this;
    }
    ContextRef& operator=(const ContextRef&) = default;
    ContextRef& operator=(ContextRef&&) = default;

    operator std::shared_ptr<Context>() const {
        return ptr.lock();
    }

    operator bool() const {
        return !ptr.expired();
    }

    std::shared_ptr<Context> operator->() const {
        return ptr.lock();
    }

    std::shared_ptr<Context> lock() const {
        return ptr.lock();
    }
};

class DataType {
public:
    optional<Position> pos_start;
    optional<Position> pos_end;
    ContextRef context;

    virtual ~DataType() = default;

    virtual DataType& set_pos(const optional<Position>& start, const optional<Position>& end) {
        this->pos_start = start;
        this->pos_end = end;
        return *this;
    }

    DataType& set_pos() { return set_pos({}, {}); }

    virtual DataType& set_context(const shared_ptr<Context>& ctx) {
        this->context = ctx;
        return *this;
    }

    DataType& set_context() { return set_context({}); }

    using OperationResult = pair<shared_ptr<DataType>, shared_ptr<RunTimeError>>;

    [[nodiscard]] virtual string get_type_name() const {
        return "DataType";
    }

    [[nodiscard]] virtual shared_ptr<DataType> copy() const = 0;
    [[nodiscard]] virtual string to_string() const = 0;

    [[nodiscard]] virtual bool is_truthy() const = 0;
    [[nodiscard]] virtual bool is_mutable() const { return false; }
    [[nodiscard]] virtual bool is_dict() const { return false; }
    [[nodiscard]] virtual bool is_callable_type() const { return false; }
    [[nodiscard]] virtual bool is_number() const { return false; }
    [[nodiscard]] virtual bool is_string() const { return false; }
    [[nodiscard]] virtual bool is_list() const { return false; }
    [[nodiscard]] virtual bool is_model_instance() const { return false; }
    [[nodiscard]] virtual bool is_function() const { return false; }
    [[nodiscard]] virtual bool is_builtin_function() const { return false; }
    [[nodiscard]] virtual bool is_bound_method() const { return false; }
    [[nodiscard]] virtual bool is_model_type() const { return false; }
    [[nodiscard]] virtual bool is_super_proxy() const { return false; }
    [[nodiscard]] virtual bool is_file() const { return false; }
    [[nodiscard]] virtual bool is_module() const { return false; }
    [[nodiscard]] virtual bool is_null() const { return false; }

    [[nodiscard]] virtual OperationResult is_true() const = 0;
    [[nodiscard]] virtual OperationResult add(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult subtract(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult multiply(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult divide(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult modulus(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult exponent(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult floor_divide(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult get_comparison_eq(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult get_comparison_neq(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult get_comparison_lt(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult get_comparison_gt(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult get_comparison_lte(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult get_comparison_gte(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult and_by(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult or_by(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult not_by() const = 0;
    [[nodiscard]] virtual OperationResult bitwise_and(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult bitwise_xor(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult bitwise_or(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult bitwise_not() const = 0;
    [[nodiscard]] virtual OperationResult lshift(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult rshift(const shared_ptr<DataType>& other) const = 0;

    [[nodiscard]] virtual OperationResult getByIndex(const vector<shared_ptr<DataType>>& indexes, const Position& pos_start = Position(), const Position& pos_end = Position()) const {
        return { nullptr, make_shared<RunTimeError>(pos_start, pos_end, "Type '" + get_type_name() + "' is not scriptable/indexable", context) };
    }

    [[nodiscard]] virtual OperationResult assignIndex(const vector<shared_ptr<DataType>>& indexes, const shared_ptr<DataType>& value, const Position& pos_start = Position(), const Position& pos_end = Position()) const {
        return { nullptr, make_shared<RunTimeError>(pos_start, pos_end, "Type '" + get_type_name() + "' does not support index assignment", context) };
    }

    [[nodiscard]] virtual OperationResult get_attr(const string& attr_name, const shared_ptr<Context>& calling_context) const {
        return { nullptr, make_shared<AttributeError>(
            pos_start.value_or(Position()), pos_end.value_or(Position()),
            "'" + get_type_name() + "' object has no attribute '" + attr_name + "'",
            calling_context) };
    }
};

inline pair<shared_ptr<DataType>, shared_ptr<RunTimeError>> illegal_op_for_bound(const DataType* self) {
    return { nullptr, make_shared<RunTimeError>(
        self->pos_start.value_or(Position()),
        self->pos_end.value_or(Position()),
        "Illegal Operation for bound method",
        self->context
    ) };
}

class BoundMethod final : public DataType {
public:
    string name;
    shared_ptr<DataType> instance;
    std::function<OperationResult(const shared_ptr<DataType>& self, const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context)> execute_impl;

    BoundMethod(string name, shared_ptr<DataType> inst, decltype(execute_impl) impl)
        : name(std::move(name)), instance(std::move(inst)), execute_impl(std::move(impl)) {}

    [[nodiscard]] OperationResult execute(const vector<shared_ptr<DataType>>& args, const map<string, shared_ptr<DataType>>& kw_args, const shared_ptr<Context>& context) const {
        return this->execute_impl(this->instance, args, kw_args, context);
    }

    [[nodiscard]] string get_type_name() const override {
        return "BoundMethod";
    }

    [[nodiscard]] bool is_bound_method() const override { return true; }

    [[nodiscard]] bool is_truthy() const override { return true; }
    [[nodiscard]] OperationResult is_true() const override;
    [[nodiscard]] shared_ptr<DataType> copy() const override;

    [[nodiscard]] string to_string() const override {
        return "<bound method " + this->name + ">";
    }

    [[nodiscard]] OperationResult add(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult divide(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType>& other) const override;
    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType>& other) const override;
    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult not_by() const override { return illegal_op_for_bound(this); }

    [[nodiscard]] OperationResult bitwise_and(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult bitwise_xor(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult bitwise_or(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult bitwise_not() const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult lshift(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
    [[nodiscard]] OperationResult rshift(const shared_ptr<DataType>& other) const override { return illegal_op_for_bound(this); }
};