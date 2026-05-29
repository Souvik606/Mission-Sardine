#pragma once
#include <vector>
#include <memory>
#include "node.h"
#include "../language_core/lexer.h"

using namespace std;

class ForEachLoopNode final : public Node {
public:
    vector<Token> var_name_tokens;
    shared_ptr<Node> collection_node;
    shared_ptr<Node> body_node;

    ForEachLoopNode(vector<Token> var_name_tokens,
                    shared_ptr<Node> collection_node,
                    shared_ptr<Node> body_node)
        : Node(var_name_tokens.empty() ? nullopt : var_name_tokens[0].pos_start,
               body_node ? body_node->pos_end : nullopt),
          var_name_tokens(std::move(var_name_tokens)),
          collection_node(std::move(collection_node)),
          body_node(std::move(body_node)) {}

    [[nodiscard]] string to_string() const override {
        return "<foreach node>";
    }
};
