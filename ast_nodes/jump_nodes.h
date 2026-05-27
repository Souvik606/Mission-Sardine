#pragma once

#include <bits/stdc++.h>
#include "node.h"
#include "../language_core/position.h"

using namespace std;

class ReturnNode : public Node {
public:
    shared_ptr<Node> node_to_return;

    explicit ReturnNode(shared_ptr<Node> node_to_return, optional<Position> pos_start, optional<Position> pos_end)
        : Node(pos_start, pos_end), node_to_return(std::move(node_to_return)) {
    }

    [[nodiscard]] std::string to_string() const override {
        return "(YIELD " + (node_to_return ? node_to_return->to_string() : "null") + ")";
    }
};

class ContinueNode : public Node {
public:
    explicit ContinueNode(optional<Position> pos_start, optional<Position> pos_end)
        : Node(pos_start, pos_end) {
    }

    [[nodiscard]] std::string to_string() const override {
        return "(PROCEED)";
    }
};

class BreakNode : public Node {
public:
    explicit BreakNode(optional<Position> pos_start, optional<Position> pos_end)
        : Node(pos_start, pos_end) {
    }

    [[nodiscard]] std::string to_string() const override {
        return "(ESCAPE)";
    }
};
