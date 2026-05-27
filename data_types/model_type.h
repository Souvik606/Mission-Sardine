#pragma once

#include <bits/stdc++.h>
#include "data_type.h"
#include "number_type.h"
#include "../language_core/symbol_table.h"
#include "../language_core/error.h"
#include "../ast_nodes/node.h"
#include "../ast_results/runtime_result.h"

using namespace std;

class Interpreter;
class ModelInstance;
class FunctionDefinitionNode;

class ModelType final : public DataType, public enable_shared_from_this<ModelType>
{
public:
    string name;

    vector<shared_ptr<Node>> attr_node_list;

    shared_ptr<Node> init_node;

    unordered_map<string, shared_ptr<Node>> method_nodes;

    ModelType(string n,
              vector<shared_ptr<Node>> attr_ns,
              shared_ptr<Node> init_n,
              unordered_map<string, shared_ptr<Node>> methods)
        : name(std::move(n)),
          attr_node_list(std::move(attr_ns)),
          init_node(std::move(init_n)),
          method_nodes(std::move(methods))
    {
    }

    [[nodiscard]] bool is_truthy() const override { return true; }

    [[nodiscard]] OperationResult is_true() const override
    {
        auto r = make_shared<Number>(1LL);
        r->set_context(context).set_pos(pos_start, pos_end);
        return {r, nullopt};
    }

    [[nodiscard]] shared_ptr<DataType> copy() const override
    {
        auto c = make_shared<ModelType>(name, attr_node_list, init_node, method_nodes);
        c->set_context(context).set_pos(pos_start, pos_end);
        return c;
    }

    [[nodiscard]] string to_string() const override
    {
        return "<model " + name + ">";
    }

    RunTimeResult execute(const vector<shared_ptr<DataType>> &args, Interpreter &interp);

    [[nodiscard]] OperationResult add(const shared_ptr<DataType> &o) const override { return err("'+'"); }
    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType> &o) const override { return err("'-'"); }
    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType> &o) const override { return err("'*'"); }
    [[nodiscard]] OperationResult divide(const shared_ptr<DataType> &o) const override { return err("'/'"); }
    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType> &o) const override { return err("'%'"); }
    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType> &o) const override { return err("'**'"); }
    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType> &o) const override { return err("'//'"); }
    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType> &o) const override { return err("'=='"); }
    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType> &o) const override { return err("'!='"); }
    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType> &o) const override { return err("'<'"); }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType> &o) const override { return err("'>'"); }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType> &o) const override { return err("'<='"); }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType> &o) const override { return err("'>='"); }
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType> &o) const override { return err("'and'"); }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType> &o) const override { return err("'or'"); }
    [[nodiscard]] OperationResult not_by() const override { return err("'not'"); }

private:
    [[nodiscard]] OperationResult err(const string &op) const
    {
        return {nullptr, IllegalOperationError(
                             pos_start.value_or(Position()), pos_end.value_or(Position()),
                             "Cannot apply " + op + " to a model", context)};
    }
};

class ModelInstance final : public DataType, public enable_shared_from_this<ModelInstance>
{
public:
    shared_ptr<ModelType> model;
    shared_ptr<SymbolTable> symbol_table;
    explicit ModelInstance(shared_ptr<ModelType> m)
        : model(std::move(m)),
          symbol_table(make_shared<SymbolTable>())
    {
    }

    OperationResult get_attr(const string &attr_name, Interpreter &interp) const;

    OperationResult set_attr(const string &attr_name, shared_ptr<DataType> value)
    {
        symbol_table->set(attr_name, std::move(value));
        return {nullptr, nullopt};
    }

    [[nodiscard]] bool is_truthy() const override { return true; }

    [[nodiscard]] OperationResult is_true() const override
    {
        auto r = make_shared<Number>(1LL);
        r->set_context(context).set_pos(pos_start, pos_end);
        return {r, nullopt};
    }

    [[nodiscard]] shared_ptr<DataType> copy() const override
    {
        auto c = make_shared<ModelInstance>(model);
        c->symbol_table = symbol_table;
        c->set_context(context).set_pos(pos_start, pos_end);
        return c;
    }

    [[nodiscard]] string to_string() const override
    {
        return "<instance of " + model->name + ">";
    }

    [[nodiscard]] OperationResult add(const shared_ptr<DataType> &o) const override { return err("'+'"); }
    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType> &o) const override { return err("'-'"); }
    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType> &o) const override { return err("'*'"); }
    [[nodiscard]] OperationResult divide(const shared_ptr<DataType> &o) const override { return err("'/'"); }
    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType> &o) const override { return err("'%'"); }
    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType> &o) const override { return err("'**'"); }
    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType> &o) const override { return err("'//'"); }
    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType> &o) const override { return err("'=='"); }
    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType> &o) const override { return err("'!='"); }
    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType> &o) const override { return err("'<'"); }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType> &o) const override { return err("'>'"); }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType> &o) const override { return err("'<='"); }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType> &o) const override { return err("'>='"); }
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType> &o) const override { return err("'and'"); }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType> &o) const override { return err("'or'"); }
    [[nodiscard]] OperationResult not_by() const override { return err("'not'"); }

private:
    [[nodiscard]] OperationResult err(const string &op) const
    {
        return {nullptr, IllegalOperationError(
                             pos_start.value_or(Position()), pos_end.value_or(Position()),
                             "Cannot apply " + op + " to a model instance", context)};
    }
};
