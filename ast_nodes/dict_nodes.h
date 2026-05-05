#pragma once
#include "node.h"

using namespace std;

class DictNode final : public Node {
public:
    vector<pair<shared_ptr<Node>, shared_ptr<Node>>> keyval_nodes;

    DictNode(vector<pair<shared_ptr<Node>, shared_ptr<Node>>> keyval_nodes, Position pos_start, Position pos_end)
        : Node(std::move(pos_start), std::move(pos_end)), keyval_nodes(std::move(keyval_nodes)) {}

    [[nodiscard]] string to_string() const override {
        stringstream ss;
        ss << "{";
        bool first = true;
        for (const auto& pair : keyval_nodes) {
            if (!first) ss << ", ";
            first = false;
            ss << pair.first->to_string() << ": " << pair.second->to_string();
        }
        ss << "}";
        return ss.str();
    }
};