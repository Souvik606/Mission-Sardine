#pragma once

#include "node.h"
#include "../language_core/lexer.h"
#include <memory>
#include <vector>
#include <string>

using namespace std;

class ListComprehensionNode final : public Node {
public:
    shared_ptr<Node> expr_node;
    string loop_type; // "cycle" or "trace"

    // Cycle specific fields
    optional<Token> var_name_tok;
    shared_ptr<Node> start_node;
    shared_ptr<Node> end_node;
    shared_ptr<Node> step_node;

    // trace specific fields
    vector<Token> var_name_tokens;
    shared_ptr<Node> collection_node;

    // shared condition filter
    shared_ptr<Node> condition_node;

    ListComprehensionNode(
        shared_ptr<Node> expr_node,
        string loop_type,
        optional<Token> var_name_tok = nullopt,
        shared_ptr<Node> start_node = nullptr,
        shared_ptr<Node> end_node = nullptr,
        shared_ptr<Node> step_node = nullptr,
        vector<Token> var_name_tokens = {},
        shared_ptr<Node> collection_node = nullptr,
        shared_ptr<Node> condition_node = nullptr,
        optional<Position> pos_start = nullopt,
        optional<Position> pos_end = nullopt
    ) : Node(std::move(pos_start), std::move(pos_end)),
        expr_node(std::move(expr_node)),
        loop_type(std::move(loop_type)),
        var_name_tok(std::move(var_name_tok)),
        start_node(std::move(start_node)),
        end_node(std::move(end_node)),
        step_node(std::move(step_node)),
        var_name_tokens(std::move(var_name_tokens)),
        collection_node(std::move(collection_node)),
        condition_node(std::move(condition_node)) {
    }

    [[nodiscard]] string to_string() const override {
        return "ListComp(" + loop_type + ")";
    }
};

class DictComprehensionNode final : public Node {
public:
    shared_ptr<Node> key_node;
    shared_ptr<Node> val_node;
    string loop_type; // "cycle" or "trace"

    // Cycle specific fields
    optional<Token> var_name_tok;
    shared_ptr<Node> start_node;
    shared_ptr<Node> end_node;
    shared_ptr<Node> step_node;

    // trace specific fields
    vector<Token> var_name_tokens;
    shared_ptr<Node> collection_node;

    // shared condition filter
    shared_ptr<Node> condition_node;

    DictComprehensionNode(
        shared_ptr<Node> key_node,
        shared_ptr<Node> val_node,
        string loop_type,
        optional<Token> var_name_tok = nullopt,
        shared_ptr<Node> start_node = nullptr,
        shared_ptr<Node> end_node = nullptr,
        shared_ptr<Node> step_node = nullptr,
        vector<Token> var_name_tokens = {},
        shared_ptr<Node> collection_node = nullptr,
        shared_ptr<Node> condition_node = nullptr,
        optional<Position> pos_start = nullopt,
        optional<Position> pos_end = nullopt
    ) : Node(std::move(pos_start), std::move(pos_end)),
        key_node(std::move(key_node)),
        val_node(std::move(val_node)),
        loop_type(std::move(loop_type)),
        var_name_tok(std::move(var_name_tok)),
        start_node(std::move(start_node)),
        end_node(std::move(end_node)),
        step_node(std::move(step_node)),
        var_name_tokens(std::move(var_name_tokens)),
        collection_node(std::move(collection_node)),
        condition_node(std::move(condition_node)) {
    }

    [[nodiscard]] string to_string() const override {
        return "DictComp(" + loop_type + ")";
    }
};
