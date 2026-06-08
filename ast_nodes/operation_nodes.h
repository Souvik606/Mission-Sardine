#pragma once

#include <bits/stdc++.h>
#include "../language_core/lexer.h"
#include "node.h"

using namespace std;

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
        this->depth = 1 + node->depth;
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
        this->depth = 1 + max(left_node->depth, right_node->depth);
    }

    [[nodiscard]] std::string to_string() const override {
        return "(" + left_node->to_string() + ", " + operator_token.to_string() + ", " + right_node->to_string() + ")";
    }
};

class TernaryOperationNode final : public Node {
public:
    shared_ptr<Node> comp_node;
    shared_ptr<Node> true_node;
    shared_ptr<Node> false_node;

    explicit TernaryOperationNode(shared_ptr<Node> comp, shared_ptr<Node> true_n, shared_ptr<Node> false_n)
        : Node(comp->pos_start, false_n->pos_end),
        comp_node(std::move(comp)),
        true_node(std::move(true_n)),
        false_node(std::move(false_n)) {
        this->depth = 1 + max({comp_node->depth, true_node->depth, false_node->depth});
    }

    [[nodiscard]] std::string to_string() const override {
        return "(" + comp_node->to_string() + " ? " + true_node->to_string() + " : " + false_node->to_string() + ")";
    }
};
