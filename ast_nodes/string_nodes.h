#pragma once

#include <bits/stdc++.h>
#include "../language_core/lexer.h"
#include "node.h"

class StringNode final : public Node {
public:
    Token token;

    explicit StringNode(Token token)
        : Node(token.pos_start, token.pos_end), token(std::move(token)) {
    }

    [[nodiscard]] int get_node_type() const override {
        return NODE_STRING;
    }

    [[nodiscard]] std::string to_string() const override {
        return token.to_string();
    }
};