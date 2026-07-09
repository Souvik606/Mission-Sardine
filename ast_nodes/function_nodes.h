#pragma once

#include <bits/stdc++.h>

#include "../language_core/lexer.h"
#include "node.h"

using namespace std;

class FunctionDefinitionNode final : public Node {
public:
    optional<Token> var_name_tok;
    vector<pair<Token, shared_ptr<Node>>> arg_nodes;
    shared_ptr<Node> body_node;
    bool return_null;
    string access_modifier; 

    explicit FunctionDefinitionNode(
        optional<Token> var_name,
        vector<pair<Token, shared_ptr<Node>>> args,
        shared_ptr<Node> body,
        bool return_null,
        string access_mod = ""
    )
        : Node(
            [&]() {
                if (var_name.has_value()) return var_name->pos_start;
                if (!args.empty()) return args.front().first.pos_start;
                return body->pos_start;
            }(),
                body->pos_end
                ),
        var_name_tok(std::move(var_name)),
        arg_nodes(std::move(args)),
        body_node(std::move(body)),
        return_null(return_null),
        access_modifier(std::move(access_mod))
    {
    }

    [[nodiscard]] std::string to_string() const override {
        const string name = var_name_tok.has_value() ? any_cast<string>(var_name_tok->value) : "<anonymous>";
        stringstream ss;
        ss << "(FUNCTION " << name << "(";
        for (size_t i = 0; i < arg_nodes.size(); ++i) {
            ss << any_cast<string>(arg_nodes[i].first.value);
            if (arg_nodes[i].second) {
                ss << "=" << arg_nodes[i].second->to_string();
            }
            if (i < arg_nodes.size() - 1) ss << ", ";
        }
        ss << ") -> " << (body_node ? body_node->to_string() : "null") << ")";
        return ss.str();
    }
};

class FunctionCallNode final : public Node {
public:
    shared_ptr<Node> call_node;
    vector<shared_ptr<Node>> positional_arg_nodes;
    vector<pair<Token, shared_ptr<Node>>> keyword_arg_nodes;

    explicit FunctionCallNode(
        shared_ptr<Node> to_call,
        vector<shared_ptr<Node>> pos_args,
        vector<pair<Token, shared_ptr<Node>>> kw_args = {},
        optional<Position> custom_pos_end = nullopt
    )
        : Node(
            to_call->pos_start,
            custom_pos_end.has_value() ? custom_pos_end : [&]() {
                if (!kw_args.empty()) return kw_args.back().second->pos_end;
                if (!pos_args.empty()) return pos_args.back()->pos_end;
                return to_call->pos_end;
            }()
        ),
        call_node(std::move(to_call)),
        positional_arg_nodes(std::move(pos_args)),
        keyword_arg_nodes(std::move(kw_args))
    {
    }

    [[nodiscard]] std::string to_string() const override {
        stringstream ss;
        ss << "(CALL " << (call_node ? call_node->to_string() : "null") << "(";
        for (size_t i = 0; i < positional_arg_nodes.size(); ++i) {
            ss << (positional_arg_nodes[i] ? positional_arg_nodes[i]->to_string() : "null");
            if (i < positional_arg_nodes.size() - 1 || !keyword_arg_nodes.empty()) ss << ", ";
        }
        for (size_t i = 0; i < keyword_arg_nodes.size(); ++i) {
            ss << any_cast<string>(keyword_arg_nodes[i].first.value) << "=" 
               << (keyword_arg_nodes[i].second ? keyword_arg_nodes[i].second->to_string() : "null");
            if (i < keyword_arg_nodes.size() - 1) ss << ", ";
        }
        ss << "))";
        return ss.str();
    }
};