#pragma once
#include <string>
#include <vector>
#include <memory>
#include <any>
#include "node.h"

using namespace std;

class FStringNode final : public Node {
public:
    // pair of kind ("literal" or "expr") and value (literal string or shared_ptr<Node>)
    vector<pair<string, any>> parts;

    FStringNode(vector<pair<string, any>> parts,
                optional<Position> pos_start = nullopt,
                optional<Position> pos_end = nullopt)
        : Node(std::move(pos_start), std::move(pos_end)),
          parts(std::move(parts)) {}

    [[nodiscard]] string to_string() const override {
        return "<fstring node>";
    }
};
