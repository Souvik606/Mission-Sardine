#pragma once
#include <string>
#include <memory>
#include "data_type.h"
#include "number_type.h"
#include "../language_core/symbol_table.h"
#include "../language_core/error.h"

using namespace std;

class Interpreter;

class Module final : public DataType {
public:
    string name;
    shared_ptr<SymbolTable> symbol_table;

    Module(string name, shared_ptr<SymbolTable> symbol_table)
        : name(std::move(name)), symbol_table(std::move(symbol_table)) {}

    [[nodiscard]] bool is_callable_type() const override { return true; }

    [[nodiscard]] bool is_truthy() const override {
        return true;
    }

    [[nodiscard]] OperationResult is_true() const override {
        auto result = make_shared<Number>(1LL);
        result->set_context(this->context);
        result->set_pos(this->pos_start, this->pos_end);
        return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
    }

    [[nodiscard]] shared_ptr<DataType> copy() const override {
        auto c = make_shared<Module>(name, symbol_table);
        c->set_pos(pos_start, pos_end);
        c->set_context(context);
        return c;
    }

    [[nodiscard]] string to_string() const override {
        return "<module '" + name + "'>";
    }

    [[nodiscard]] OperationResult get_attr(const string& attr_name, const shared_ptr<Context>& calling_context) const override {
        auto value = symbol_table->get(attr_name);
        if (!value) {
            return { nullptr, make_shared<AttributeError>(
                pos_start.value_or(Position()), pos_end.value_or(Position()),
                "Module '" + name + "' has no member '" + attr_name + "'",
                calling_context
            ) };
        }
        return { value, nullptr };
    }

    // Binary / Unary operators (delegated error cases since operators are illegal on modules)
    [[nodiscard]] OperationResult add(const shared_ptr<DataType> &o) const override { return err("'+'"); }
    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType> &o) const override { return err("'-'"); }
    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType> &o) const override { return err("'*'"); }
    [[nodiscard]] OperationResult divide(const shared_ptr<DataType> &o) const override { return err("'/'"); }
    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType> &o) const override { return err("'%'"); }
    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType> &o) const override { return err("'**'"); }
    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType> &o) const override { return err("'//'"); }
    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType>& other) const override {
        if (other->get_type_name() == "Null") {
            return { Number::make_bool(false), nullptr };
        }
        bool eq = (this == other.get());
        return { Number::make_bool(eq), nullptr };
    }
    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType>& other) const override {
        if (other->get_type_name() == "Null") {
            return { Number::make_bool(true), nullptr };
        }
        bool neq = (this != other.get());
        return { Number::make_bool(neq), nullptr };
    }
    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType> &o) const override { return err("'<'"); }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType> &o) const override { return err("'>'"); }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType> &o) const override { return err("'<='"); }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType> &o) const override { return err("'>='"); }
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType> &o) const override { return err("'and'"); }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType> &o) const override { return err("'or'"); }
    [[nodiscard]] OperationResult not_by() const override { return err("'not'"); }

    [[nodiscard]] OperationResult bitwise_and(const shared_ptr<DataType> &o) const override { return err("'&'"); }
    [[nodiscard]] OperationResult bitwise_xor(const shared_ptr<DataType> &o) const override { return err("'^'"); }
    [[nodiscard]] OperationResult bitwise_or(const shared_ptr<DataType> &o) const override { return err("'|'"); }
    [[nodiscard]] OperationResult bitwise_not() const override { return err("'~'"); }
    [[nodiscard]] OperationResult lshift(const shared_ptr<DataType> &o) const override { return err("'<<'"); }
    [[nodiscard]] OperationResult rshift(const shared_ptr<DataType> &o) const override { return err("'>>'"); }

private:
    [[nodiscard]] OperationResult err(const string &op) const
    {
        return {nullptr, make_shared<IllegalOperationError>(
                             pos_start.value_or(Position()), pos_end.value_or(Position()),
                             "Cannot apply " + op + " to a module", context)};
    }
};
