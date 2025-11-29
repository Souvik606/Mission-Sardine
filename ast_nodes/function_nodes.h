#pragma once

#include <bits/stdc++.h>

#include "../language_core/lexer.h"
#include "node.h"

using namespace std;

class FunctionDefinitionNode final : public Node {
public:
    optional<Token> var_name_tok;
    vector<Token> arg_name_toks;
    shared_ptr<Node> body_node;
    bool return_null;

    explicit FunctionDefinitionNode(
        optional<Token> var_name,
        vector<Token> arg_names,
        shared_ptr<Node> body,
        bool return_null
    )
        : Node(
            [&]() {
                if (var_name.has_value()) return var_name->pos_start;
                if (!arg_names.empty()) return arg_names.front().pos_start;
                return body->pos_start;
            }(),
                body->pos_end
                ),
        var_name_tok(std::move(var_name)),
        arg_name_toks(std::move(arg_names)),
        body_node(std::move(body)),
        return_null(return_null)
    {
    }

    [[nodiscard]] std::string to_string() const override {
        const string name = var_name_tok.has_value() ? any_cast<string>(var_name_tok->value) : "<anonymous>";
        return "(FUNCTION " + name + ")";
    }
};

class FunctionCallNode final : public Node {
public:
    shared_ptr<Node> call_node;
    vector<shared_ptr<Node>> arg_nodes;

    explicit FunctionCallNode(
        shared_ptr<Node> to_call,
        vector<shared_ptr<Node>> args
    )
        : Node(
            to_call->pos_start,
            args.empty() ? to_call->pos_end : args.back()->pos_end
        ),
        call_node(std::move(to_call)),
        arg_nodes(std::move(args))
    {
    }

    [[nodiscard]] std::string to_string() const override {
        return "(CALL)";
    }
};