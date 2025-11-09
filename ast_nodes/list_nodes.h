#pragma once

#include <bits/stdc++.h>

#include "node.h"
#include "../language_core/position.h"

using namespace std;

class ListNode final : public Node {
public:
    vector<shared_ptr<Node>> element_nodes;

    explicit ListNode(
        vector<shared_ptr<Node>> elements,
        optional<Position> pos_start,
        optional<Position> pos_end
    )
        : Node(std::move(pos_start), std::move(pos_end)),
          element_nodes(std::move(elements))
    {}

    [[nodiscard]] std::string to_string() const override {
        stringstream ss;
        ss << "[";
        for (size_t i = 0; i < element_nodes.size(); ++i) {
            if (element_nodes[i]) {
                ss << element_nodes[i]->to_string();
            } else {
                ss << "<null_node>";
            }
            if (i < element_nodes.size() - 1) {
                ss << ", ";
            }
        }
        ss << "]";
        return ss.str();
    }
};