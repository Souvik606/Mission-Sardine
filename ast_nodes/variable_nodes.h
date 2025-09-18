#pragma once

#include <bits/stdc++.h>
#include "../language_core/lexer.h"
#include "node.h"

using namespace std;

class VariableUseNode final : public Node {
public:
    Token var_name_tok;

    explicit VariableUseNode(Token token)
        : Node(token.pos_start, token.pos_end),
          var_name_tok(std::move(token)) {}

    [[nodiscard]] std::string to_string() const override {
        if (var_name_tok.value.type() == typeid(string)) {
            return "(" + any_cast<string>(var_name_tok.value) + ")";
        }
        return "(invalid_variable_name)";
    }
};

class VariableAssignNode final : public Node {
public:
    Token var_name_tok;
    shared_ptr<Node> value_node;

    explicit VariableAssignNode(Token token, shared_ptr<Node> value)
        : Node(token.pos_start, value->pos_end),
          var_name_tok(std::move(token)),
          value_node(std::move(value)) {}

    [[nodiscard]] std::string to_string() const override {
        stringstream ss;
        string var_name = "invalid_variable_name";
        if (var_name_tok.value.type() == typeid(string)) {
            var_name = any_cast<string>(var_name_tok.value);
        }
        ss << "(" << var_name << ":" << value_node->to_string() << ")";
        return ss.str();
    }
};