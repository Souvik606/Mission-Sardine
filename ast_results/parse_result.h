#pragma once
#include <bits/stdc++.h>
#include "../ast_nodes/operation_nodes.h"

using namespace std;

class ParseResult {
public:
    optional<Error> error;
    shared_ptr<Node> node;

    ParseResult() : error(nullopt), node(nullptr) {}

    shared_ptr<Node> register_node(const ParseResult& res) {
        if (res.error) {
            this->error = res.error;
        }
        return res.node;
    }

    static shared_ptr<Node> register_node(shared_ptr<Node> res_node) {
        return res_node;
    }

    ParseResult& success(shared_ptr<Node> success_node) {
        this->node = std::move(success_node);
        return *this;
    }

    ParseResult& failure(const Error& failure_error) {
        this->error = failure_error;
        return *this;
    }
};