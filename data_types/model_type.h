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

struct AttrInfo {
    string name;
    shared_ptr<Node> default_node;
    string access_modifier;
};

struct MethodInfo {
    shared_ptr<Node> node;
    string access_modifier;
};

class ModelType final : public DataType, public enable_shared_from_this<ModelType>
{
public:
    string name;

    vector<AttrInfo> own_attributes;

    vector<shared_ptr<Node>> attr_node_list;

    shared_ptr<Node> init_node;

    unordered_map<string, MethodInfo> own_method_nodes;
    vector<shared_ptr<ModelType>> parents;

    ModelType(string n,
              vector<AttrInfo> own_attrs,
              vector<shared_ptr<Node>> attr_ns,
              shared_ptr<Node> init_n,
              unordered_map<string, MethodInfo> methods,
              vector<shared_ptr<ModelType>> parents = {})
        : name(std::move(n)),
          own_attributes(std::move(own_attrs)),
          attr_node_list(std::move(attr_ns)),
          init_node(std::move(init_n)),
          own_method_nodes(std::move(methods)),
          parents(std::move(parents))
    {
    }

    const MethodInfo* find_method(const string& mname) const
    {
        auto it = own_method_nodes.find(mname);
        if (it != own_method_nodes.end())
            return &it->second;
        for (const auto& p : parents)
        {
            const MethodInfo* found = p->find_method(mname);
            if (found) return found;
        }
        return nullptr;
    }

    // Find which ModelType owns a method
    shared_ptr<ModelType> find_method_owner(const string& mname)
    {
        if (own_method_nodes.count(mname))
            return shared_from_this();
        for (const auto& p : parents)
        {
            auto owner = p->find_method_owner(mname);
            if (owner) return owner;
        }
        return nullptr;
    }

    const AttrInfo* find_attribute(const string& aname) const
    {
        for (const auto& ai : own_attributes)
            if (ai.name == aname) return &ai;
        for (const auto& p : parents)
        {
            const AttrInfo* found = p->find_attribute(aname);
            if (found) return found;
        }
        return nullptr;
    }

    shared_ptr<ModelType> find_attribute_owner(const string& aname)
    {
        for (const auto& ai : own_attributes)
            if (ai.name == aname) return shared_from_this();
        for (const auto& p : parents)
        {
            auto owner = p->find_attribute_owner(aname);
            if (owner) return owner;
        }
        return nullptr;
    }

    bool is_descendant_of(const shared_ptr<ModelType>& other_model) const
    {
        if (this->name == other_model->name) return true;
        for (const auto& p : parents)
            if (p->is_descendant_of(other_model)) return true;
        return false;
    }

    vector<shared_ptr<Node>> all_attr_nodes() const
    {
        vector<shared_ptr<Node>> result;
        for (const auto& p : parents)
        {
            auto parent_attrs = p->all_attr_nodes();
            result.insert(result.end(), parent_attrs.begin(), parent_attrs.end());
        }
        result.insert(result.end(), attr_node_list.begin(), attr_node_list.end());
        return result;
    }

    [[nodiscard]] bool is_truthy() const override { return true; }

    [[nodiscard]] OperationResult is_true() const override
    {
        auto r = make_shared<Number>(1LL);
        r->set_context(context).set_pos(pos_start, pos_end);
        return {r, nullptr};
    }

    [[nodiscard]] shared_ptr<DataType> copy() const override
    {
        auto c = make_shared<ModelType>(name, own_attributes, attr_node_list, init_node, own_method_nodes, parents);
        c->set_context(context).set_pos(pos_start, pos_end);
        return c;
    }

    [[nodiscard]] string to_string() const override
    {
        return "<model " + name + ">";
    }

    RunTimeResult execute(const vector<shared_ptr<DataType>> &pos_args, const map<string, shared_ptr<DataType>> &kw_args, Interpreter &interp);

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

    OperationResult get_attr(const string &attr_name, Interpreter &interp,
                             const shared_ptr<Context> &calling_context) const;

    OperationResult set_attr(const string &attr_name, shared_ptr<DataType> value)
    {
        symbol_table->set(attr_name, std::move(value));
        return {nullptr, nullptr};
    }

    [[nodiscard]] bool is_truthy() const override { return true; }

    [[nodiscard]] OperationResult is_true() const override
    {
        auto r = make_shared<Number>(1LL);
        r->set_context(context).set_pos(pos_start, pos_end);
        return {r, nullptr};
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

    [[nodiscard]] OperationResult _call_op_method(const string &method_name, const vector<shared_ptr<DataType>> &args) const;
    [[nodiscard]] OperationResult _binary_op(const string &op_name, const shared_ptr<DataType> &other) const;
    [[nodiscard]] OperationResult _unary_op(const string &op_name) const;

    [[nodiscard]] OperationResult add(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult divide(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult not_by() const override;

    [[nodiscard]] OperationResult bitwise_and(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult bitwise_xor(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult bitwise_or(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult bitwise_not() const override;
    [[nodiscard]] OperationResult lshift(const shared_ptr<DataType> &o) const override;
    [[nodiscard]] OperationResult rshift(const shared_ptr<DataType> &o) const override;

private:
    [[nodiscard]] OperationResult err(const string &op) const;
};
