#pragma once
#include <bits/stdc++.h>
#include "../lexer.h"

using namespace std;

class Node {
public:
    virtual ~Node() {}
    virtual std::string to_string() const { return ""; }
};

class NumberNode final : public Node {
public:
    Token token;

    explicit NumberNode(const Token &token) : token(token) {}

    std::string to_string() const override {
        return token.to_string();
    }
};

class UnaryOperationNode final : public Node {
public:
    Token operator_token;
    shared_ptr<Node> node;

    explicit UnaryOperationNode(const Token &op, shared_ptr<Node> n)
        : operator_token(op), node(std::move(n)) {}

    std::string to_string() const override {
        return "(" + operator_token.to_string() + ", " + node->to_string() + ")";
    }
};

class BinaryOperationNode final : public Node {
public:
    shared_ptr<Node> left_node;
    Token operator_token;
    shared_ptr<Node> right_node;

    explicit BinaryOperationNode(shared_ptr<Node> left, const Token &op, shared_ptr<Node> right)
        : left_node(std::move(left)), operator_token(op), right_node(std::move(right)) {}

    std::string to_string() const override {
        return "(" + left_node->to_string() + ", " + operator_token.to_string() + ", " + right_node->to_string() + ")";
    }
};