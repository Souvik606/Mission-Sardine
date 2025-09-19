#pragma once

#include <bits/stdc++.h>
#include "../ast_nodes/operation_nodes.h"

using namespace std;

class ParseResult {
public:
    optional<Error> error;
    shared_ptr<Node> node;
    int advance_count = 0;

    void register_advancement() {
        advance_count++;
    }

    shared_ptr<Node> register_node(const ParseResult& res) {
        this->advance_count += res.advance_count;
        if (res.error) {
            this->error = res.error;
        }
        return res.node;
    }

    ParseResult& success(shared_ptr<Node> success_node) {
        this->node = std::move(success_node);
        return *this;
    }

    ParseResult& failure(const Error& failure_error) {
        if (!this->error || this->advance_count == 0) {
            this->error = failure_error;
        }
        return *this;
    }
};
