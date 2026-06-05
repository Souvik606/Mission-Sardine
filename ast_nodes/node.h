#pragma once

#include <bits/stdc++.h>
#include "../language_core/position.h"

using namespace std;

class Node {
public:
    optional<Position> pos_start;
    optional<Position> pos_end;
    int depth = 1;

    Node(optional<Position> start, optional<Position> end)
        : pos_start(std::move(start)), pos_end(std::move(end)) {
    }

    virtual ~Node() = default;

    [[nodiscard]] virtual std::string to_string() const { return ""; }
};
