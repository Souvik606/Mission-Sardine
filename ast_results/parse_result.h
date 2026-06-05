#pragma once

#include <bits/stdc++.h>
#include "../ast_nodes/node.h"
#include "../language_core/error.h"

using namespace std;

class ParseResult {
public:
    shared_ptr<Error> error = nullptr;
    shared_ptr<Node> node;
    int advance_count = 0;
    bool is_fatal = false;

    int last_registered_advance_count = 0;
    int to_reverse_count = 0;

    void register_advancement() {
        last_registered_advance_count = 1;
        advance_count++;
    }

    void reverse(int amount = 1) {
        advance_count -= amount;
        to_reverse_count += amount;
    }

    shared_ptr<Node> register_node(const ParseResult& res) {
        last_registered_advance_count = res.advance_count;
        this->advance_count += res.advance_count;
        if (res.is_fatal) {
            this->is_fatal = true;
        }
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
            this->error = failure_error.clone();
        }
        return *this;
    }

    shared_ptr<Node> try_register(const ParseResult& res) {
        if (res.is_fatal) {
            this->error = res.error;
            this->is_fatal = true;
            return nullptr;
        }
        if (res.error) {
            this->to_reverse_count = res.advance_count;
            return nullptr;
        }
        return this->register_node(res);
    }
};
