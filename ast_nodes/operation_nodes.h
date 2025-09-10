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
    Node* node;

    explicit UnaryOperationNode(const Token &op, Node* n) : operator_token(op), node(n) {}

    std::string to_string() const override {
        return "(" + operator_token.to_string() + ", " + node->to_string() + ")";
    }
};

class BinaryOperationNode final : public Node {
public:
    Node* left_node;
    Token operator_token;
    Node* right_node;

    explicit BinaryOperationNode(Node* left, const Token &op, Node* right)
        : left_node(left), operator_token(op), right_node(right) {}

    std::string to_string() const override {
        return "(" + left_node->to_string() + ", " + operator_token.to_string() + ", " + right_node->to_string() + ")";
    }
};