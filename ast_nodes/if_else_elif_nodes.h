#pragma once

#include <bits/stdc++.h>
#include "node.h"

using namespace std;

class IfNode final : public Node {
public:
    vector<tuple<shared_ptr<Node>, shared_ptr<Node>, bool>> cases;
    optional<pair<shared_ptr<Node>, bool>> else_case;

    explicit IfNode(
        vector<tuple<shared_ptr<Node>, shared_ptr<Node>, bool>> cases_list,
        optional<pair<shared_ptr<Node>, bool>> else_node
    )
        : Node(
            get<0>(cases_list.front())->pos_start,
            (else_node.has_value() ? else_node->first->pos_end : get<1>(cases_list.back())->pos_end)
          ),
          cases(std::move(cases_list)),
          else_case(std::move(else_node))
    {}

    [[nodiscard]] string to_string() const override {
        stringstream ss;
        ss << "(IF ";
        for (size_t i = 0; i < cases.size(); ++i) {
            ss << (i > 0 ? "ELIF " : "");
            ss << "(" << get<0>(cases[i])->to_string() << " : " << get<1>(cases[i])->to_string() << ")";
        }
        if (else_case.has_value()) {
            ss << " ELSE (" << else_case->first->to_string() << ")";
        }
        ss << ")";
        return ss.str();
    }
};