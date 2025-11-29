#pragma once

#include <bits/stdc++.h>
#include "../ast_nodes/operation_nodes.h"

using namespace std;

class ParseResult {
public:
    optional<Error> error;
    shared_ptr<Node> node;
    int advance_count = 0;

    int last_registered_advance_count = 0;
    int to_reverse_count = 0;

    void register_advancement() {
        last_registered_advance_count = 1;
        advance_count++;
    }

    shared_ptr<Node> register_node(const ParseResult& res) {
        last_registered_advance_count = res.advance_count;
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

    shared_ptr<Node> try_register(const ParseResult& res) {
        if (res.error) {
            this->to_reverse_count = res.advance_count;
            return nullptr;
        }
        return this->register_node(res);
    }
};
