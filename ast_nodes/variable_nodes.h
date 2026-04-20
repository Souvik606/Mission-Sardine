#pragma once

#include <bits/stdc++.h>
#include <vector>
#include "../language_core/lexer.h"
#include "node.h"

using namespace std;

class VariableUseNode final : public Node {
public:
    Token var_name_tok;
    vector<shared_ptr<Node>> index_node;

    explicit VariableUseNode(Token token, vector<shared_ptr<Node>> indices = {})
        : Node(token.pos_start, indices.empty() ? token.pos_end : indices.back()->pos_end),
          var_name_tok(std::move(token)),
          index_node(std::move(indices)) {}

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
    vector<Token> var_name_toks;
    vector<shared_ptr<Node>> value_nodes;
    vector<vector<shared_ptr<Node>>> index_nodes;

    explicit VariableAssignNode(vector<Token> tokens,
                                vector<shared_ptr<Node>> values,
                                vector<vector<shared_ptr<Node>>> indices = {})
        : Node(tokens.front().pos_start, values.back()->pos_end),
          var_name_toks(std::move(tokens)),
          value_nodes(std::move(values)),
          index_nodes(std::move(indices)) {

        if (index_nodes.empty()) {
            index_nodes.resize(var_name_toks.size());
        }
    }

    [[nodiscard]] std::string to_string() const override {
        stringstream ss;
        for (size_t i = 0; i < var_name_toks.size(); ++i) {
            string var_name = "invalid_variable_name";
            if (var_name_toks[i].value.type() == typeid(string)) {
                var_name = any_cast<string>(var_name_toks[i].value);
            }

            if (index_nodes[i].empty()) {
                ss << "(" << var_name << ":" << value_nodes[i]->to_string() << ")";
            } else {
                ss << "((" << var_name << ":[";
                for (size_t j = 0; j < index_nodes[i].size(); ++j) {
                    ss << index_nodes[i][j]->to_string();
                    if (j < index_nodes[i].size() - 1) ss << ", ";
                }
                ss << "]):" << value_nodes[i]->to_string() << ")";
            }

            if (i < var_name_toks.size() - 1) {
                ss << ",";
            }
        }
        return ss.str();
    }
};