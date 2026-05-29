#pragma once
#include <string>
#include <vector>
#include <memory>
#include "node.h"
#include "../language_core/lexer.h"

using namespace std;

class SummonNode final : public Node {
public:
    Token module_tok;
    vector<pair<Token, optional<Token>>> names;
    optional<Token> module_alias;
    bool wildcard;

    SummonNode(Token module_tok,
               vector<pair<Token, optional<Token>>> names = {},
               optional<Token> module_alias = nullopt,
               bool wildcard = false,
               optional<Position> pos_start = nullopt,
               optional<Position> pos_end = nullopt)
        : Node(pos_start.has_value() ? pos_start : module_tok.pos_start,
               pos_end.has_value() ? pos_end : module_tok.pos_end),
          module_tok(std::move(module_tok)),
          names(std::move(names)),
          module_alias(std::move(module_alias)),
          wildcard(wildcard) {}

    [[nodiscard]] string to_string() const override {
        return "<summon node>";
    }
};
