#pragma once

#include <bits/stdc++.h>
#include "node.h"

using namespace std;

struct SwitchCaseNode : public Node {
    shared_ptr<Node> value; // nullptr for default
    shared_ptr<Node> body;
    bool return_null;

    SwitchCaseNode(shared_ptr<Node> v, shared_ptr<Node> b, bool ret_null)
        : Node(v ? v->pos_start : b->pos_start, b->pos_end),
        value(std::move(v)), body(std::move(b)), return_null(ret_null)
    {
    }

    [[nodiscard]] string to_string() const override {
        stringstream ss;
        if (value) {
            ss << "(CASE " << value->to_string() << " : " << body->to_string() << ")";
        }
        else {
            ss << "(DEFAULT " << body->to_string() << ")";
        }
        return ss.str();
    }
};

class SwitchNode final : public Node {
public:
    shared_ptr<Node> switch_value;
    vector<shared_ptr<SwitchCaseNode>> cases;
    bool return_null;

    SwitchNode(shared_ptr<Node> val, vector<shared_ptr<SwitchCaseNode>> c, bool ret_null)
        : Node(val->pos_start, c.empty() ? val->pos_end : c.back()->pos_end),
        switch_value(std::move(val)), cases(std::move(c)), return_null(ret_null)
    {
    }

    [[nodiscard]] string to_string() const override {
        stringstream ss;
        ss << "(SWITCH " << switch_value->to_string() << " { ";
        for (const auto& c : cases) {
            ss << c->to_string() << " ";
        }
        ss << "})";
        return ss.str();
    }
};
