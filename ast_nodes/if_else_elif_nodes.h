#pragma once

#include <bits/stdc++.h>
#include "node.h"

using namespace std;

class IfNode final : public Node {
public:
    vector<pair<shared_ptr<Node>, shared_ptr<Node>>> cases;
    shared_ptr<Node> else_case;

    explicit IfNode(vector<pair<shared_ptr<Node>, shared_ptr<Node>>> cases_list, shared_ptr<Node> else_node)
        : Node(
            cases_list.front().first->pos_start,
            (else_node ? else_node->pos_end : cases_list.back().second->pos_end)
          ),
          cases(std::move(cases_list)),
          else_case(std::move(else_node))
    {}

    [[nodiscard]] string to_string() const override {
        stringstream ss;
        ss << "(IF ";
        for (size_t i = 0; i < cases.size(); ++i) {
            ss << (i > 0 ? "ELIF " : "");
            ss << "(" << cases[i].first->to_string() << " : " << cases[i].second->to_string() << ")";
        }
        if (else_case) {
            ss << " ELSE (" << else_case->to_string() << ")";
        }
        ss << ")";
        return ss.str();
    }
};