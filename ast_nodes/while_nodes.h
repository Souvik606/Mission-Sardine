#pragma once

#include <bits/stdc++.h>

#include "../language_core/lexer.h"
#include "node.h"

using namespace std;

class WhileNode final : public Node {
public:
    shared_ptr<Node> condition_node;
    shared_ptr<Node> body_node;

    explicit WhileNode(
        shared_ptr<Node> condition,
        shared_ptr<Node> body
    )
        : Node(condition->pos_start, body->pos_end),
          condition_node(std::move(condition)),
          body_node(std::move(body))
    {}

    [[nodiscard]] std::string to_string() const override {
        stringstream ss;
        ss << "(WHILE " << condition_node->to_string()
           << " DO " << body_node->to_string() << ")";
        return ss.str();
    }
};