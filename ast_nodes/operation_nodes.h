#pragma once

#include <bits/stdc++.h>
#include "../language_core/lexer.h"
#include "../language_core/position.h"

using namespace std;

class Node {
public:
    optional<Position> pos_start;
    optional<Position> pos_end;

    Node(optional<Position> start, optional<Position> end)
        : pos_start(std::move(start)), pos_end(std::move(end)) {
    }

    virtual ~Node() = default;

    [[nodiscard]] virtual std::string to_string() const { return ""; }
};

class NumberNode final : public Node {
public:
    Token token;

    explicit NumberNode(Token token)
        : Node(token.pos_start, token.pos_end), token(std::move(token)) {
    }

    [[nodiscard]] std::string to_string() const override {
        return token.to_string();
    }
};

class UnaryOperationNode final : public Node {
public:
    Token operator_token;
    shared_ptr<Node> node;

    explicit UnaryOperationNode(Token op, shared_ptr<Node> n)
        : Node(op.pos_start, n->pos_end),
          operator_token(std::move(op)),
          node(std::move(n)) {
    }

    [[nodiscard]] std::string to_string() const override {
        return "(" + operator_token.to_string() + ", " + node->to_string() + ")";
    }
};

class BinaryOperationNode final : public Node {
public:
    shared_ptr<Node> left_node;
    Token operator_token;
    shared_ptr<Node> right_node;

    explicit BinaryOperationNode(shared_ptr<Node> left, Token op, shared_ptr<Node> right)
        : Node(left->pos_start, right->pos_end),
          left_node(std::move(left)),
          operator_token(std::move(op)),
          right_node(std::move(right)) {
    }

    [[nodiscard]] std::string to_string() const override {
        return "(" + left_node->to_string() + ", " + operator_token.to_string() + ", " + right_node->to_string() + ")";
    }
};
