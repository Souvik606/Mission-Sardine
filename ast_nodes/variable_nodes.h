#pragma once

#include <bits/stdc++.h>
#include <vector>
#include "../language_core/lexer.h"
#include "node.h"

using namespace std;

class SymbolTable;

class VariableUseNode final : public Node {
public:
    Token var_name_tok;
    vector<shared_ptr<Node>> index_node;
    mutable uint64_t cached_symbol_table_id = 0;
    mutable const shared_ptr<DataType>* cached_value_ptr = nullptr;

    explicit VariableUseNode(Token token, vector<shared_ptr<Node>> indices = {})
        : Node(token.pos_start, indices.empty() ? token.pos_end : indices.back()->pos_end),
          var_name_tok(std::move(token)),
          index_node(std::move(indices)),
          cached_value_ptr(nullptr) {}

    [[nodiscard]] std::string to_string() const override {
        string var_name = "invalid_variable_name";
        if (var_name_tok.value.type() == typeid(string)) {
            var_name = any_cast<string>(var_name_tok.value);
        }

        if (index_node.empty()) {
            return "(" + var_name + ")";
        } else {
            stringstream ss;
            ss << "(" << var_name << ":[";
            for (size_t i = 0; i < index_node.size(); ++i) {
                ss << index_node[i]->to_string();
                if (i < index_node.size() - 1) ss << ", ";
            }
            ss << "])";
            return ss.str();
        }
    }
};

class VariableAssignNode final : public Node {
public:
    vector<shared_ptr<Node>> left_nodes;
    vector<shared_ptr<Node>> value_nodes;

    explicit VariableAssignNode(vector<shared_ptr<Node>> lefts,
                                vector<shared_ptr<Node>> values)
        : Node(lefts.front()->pos_start.value_or(Position()), values.back()->pos_end.value_or(Position())),
          left_nodes(std::move(lefts)),
          value_nodes(std::move(values)) {}

    [[nodiscard]] std::string to_string() const override {
        stringstream ss;
        for (size_t i = 0; i < left_nodes.size(); ++i) {
            ss << "(" << left_nodes[i]->to_string() << ":" << value_nodes[i]->to_string() << ")";
            if (i < left_nodes.size() - 1) {
                ss << ",";
            }
        }
        return ss.str();
    }
};

class IndexAccessNode final : public Node {
public:
    shared_ptr<Node> object_node;
    shared_ptr<Node> index_node;

    explicit IndexAccessNode(shared_ptr<Node> obj, shared_ptr<Node> idx)
        : Node(obj->pos_start, idx->pos_end),
          object_node(std::move(obj)),
          index_node(std::move(idx)) {}

    [[nodiscard]] std::string to_string() const override {
        return "(" + object_node->to_string() + "[" + index_node->to_string() + "])";
    }
};