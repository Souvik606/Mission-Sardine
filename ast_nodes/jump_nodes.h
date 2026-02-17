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
};

class ContinueNode : public Node {
public:
    explicit ContinueNode(optional<Position> pos_start, optional<Position> pos_end)
        : Node(pos_start, pos_end) {
    }
};

class BreakNode : public Node {
public:
    explicit BreakNode(optional<Position> pos_start, optional<Position> pos_end)
        : Node(pos_start, pos_end) {
    }
};
