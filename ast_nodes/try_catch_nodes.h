#pragma once
#include "node.h"
#include "../language_core/lexer.h"

using namespace std;

class CatchNode final : public Node {
public:
    optional<Token> error_type;
    optional<Token> error_name;
    shared_ptr<Node> body_node;

    CatchNode(optional<Token> error_type, optional<Token> error_name, shared_ptr<Node> body_node)
        : Node(error_type ? error_type->pos_start.value() : body_node->pos_start, body_node->pos_end),
        error_type(std::move(error_type)), error_name(std::move(error_name)), body_node(std::move(body_node)) {
    }

    [[nodiscard]] string to_string() const override {
        stringstream ss;
        ss << "(trap";
        if (error_type) ss << " " << error_type->to_string();
        if (error_name) ss << " as " << error_name->to_string();
        ss << " { " << body_node->to_string() << " })";
        return ss.str();
    }
};

class FinallyNode final : public Node {
public:
    shared_ptr<Node> body_node;

    explicit FinallyNode(shared_ptr<Node> body_node)
        : Node(body_node->pos_start, body_node->pos_end), body_node(std::move(body_node)) {
    }

    [[nodiscard]] string to_string() const override {
        stringstream ss;
        ss << "(clean { " << body_node->to_string() << " })";
        return ss.str();
    }
};

class TryNode final : public Node {
public:
    shared_ptr<Node> body_node;
    vector<shared_ptr<CatchNode>> trap_nodes;
    shared_ptr<FinallyNode> clean_node;

    TryNode(shared_ptr<Node> body_node, vector<shared_ptr<CatchNode>> trap_nodes, shared_ptr<FinallyNode> clean_node)
        : Node(body_node->pos_start,
            clean_node ? clean_node->pos_end : (!trap_nodes.empty() ? trap_nodes.back()->pos_end : body_node->pos_end)),
        body_node(std::move(body_node)), trap_nodes(std::move(trap_nodes)), clean_node(std::move(clean_node)) {
    }

    [[nodiscard]] string to_string() const override {
        stringstream ss;
        ss << "(risk { " << body_node->to_string() << " }";
        for (const auto& trap : trap_nodes) {
            ss << " " << trap->to_string();
        }
        if (clean_node) {
            ss << " " << clean_node->to_string();
        }
        ss << ")";
        return ss.str();
    }
};