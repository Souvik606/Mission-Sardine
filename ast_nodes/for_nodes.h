#pragma once
#include <bits/stdc++.h>

#include "../language_core/lexer.h"
#include "node.h"

using namespace std;

class ForNode final : public Node {
public:
    Token var_name_tok;
    shared_ptr<Node> start_value_node;
    shared_ptr<Node> end_value_node;
    shared_ptr<Node> step_value_node;
    shared_ptr<Node> body_node;
    bool return_null;

    explicit ForNode(
        Token var_name,
        shared_ptr<Node> start_value,
        shared_ptr<Node> end_value,
        shared_ptr<Node> step_value,
        shared_ptr<Node> body,
        bool return_null
    )
        : Node(var_name.pos_start, body->pos_end),
        var_name_tok(std::move(var_name)),
        start_value_node(std::move(start_value)),
        end_value_node(std::move(end_value)),
        step_value_node(std::move(step_value)),
        body_node(std::move(body)),
        return_null(return_null)
    {
    }

    [[nodiscard]] int get_node_type() const override {
        return NODE_FOR;
    }

    [[nodiscard]] std::string to_string() const override {
        stringstream ss;
        ss << "(FOR " << any_cast<string>(var_name_tok.value)
            << " FROM " << start_value_node->to_string()
            << " TO " << end_value_node->to_string();

        if (step_value_node) {
            ss << " STEP " << step_value_node->to_string();
        }

        ss << " DO " << body_node->to_string() << ")";
        return ss.str();
    }
};