#pragma once

#include <bits/stdc++.h>

#include "node.h"
#include "../language_core/position.h"

using namespace std;

class ReturnNode final : public Node {
public:
    shared_ptr<Node> node_to_return;

    explicit ReturnNode(
        shared_ptr<Node> node_to_return,
        optional<Position> pos_start,
        optional<Position> pos_end
    )
        : Node(std::move(pos_start), std::move(pos_end)),
          node_to_return(std::move(node_to_return))
    {}

    [[nodiscard]] std::string to_string() const override {
        stringstream ss;
        ss << "(Yield";
        if (node_to_return) {
            ss << " " << node_to_return->to_string();
        }
        ss << ")";
        return ss.str();
    }
};

class ContinueNode final : public Node {
public:
    explicit ContinueNode(
        optional<Position> pos_start,
        optional<Position> pos_end
    )
        : Node(std::move(pos_start), std::move(pos_end))
    {}

    [[nodiscard]] std::string to_string() const override {
        return "(Proceed)";
    }
};

class BreakNode final : public Node {
public:
    explicit BreakNode(
        optional<Position> pos_start,
        optional<Position> pos_end
    )
        : Node(std::move(pos_start), std::move(pos_end))
    {}

    [[nodiscard]] std::string to_string() const override {
        return "(Escape)";
    }
};