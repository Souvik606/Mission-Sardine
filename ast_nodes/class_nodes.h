#pragma once

#include <bits/stdc++.h>
#include "node.h"
#include "../language_core/lexer.h"

using namespace std;

class ModelNode final : public Node
{
public:
    Token name_tok;
    vector<Token> parent_name_toks;   // parent class names (inheritance)
    vector<shared_ptr<Node>> body_nodes;

    ModelNode(Token name_tok, vector<Token> parents, vector<shared_ptr<Node>> body)
        : Node(name_tok.pos_start.value_or(Position()),
               body.empty() ? name_tok.pos_end.value_or(Position())
                            : body.back()->pos_end.value_or(Position())),
          name_tok(std::move(name_tok)),
          parent_name_toks(std::move(parents)),
          body_nodes(std::move(body)) {}

    [[nodiscard]] string to_string() const override
    {
        return "(Model: " + any_cast<string>(name_tok.value) + ")";
    }
};

using AttrDecl = pair<Token, shared_ptr<Node>>;

class AttrNode final : public Node
{
public:
    vector<AttrDecl> declarations;
    string access_modifier;  // "", "open", "secret", "guarded"

    AttrNode(vector<AttrDecl> decls, string access_mod, Position ps, Position pe)
        : Node(ps, pe),
          declarations(std::move(decls)),
          access_modifier(std::move(access_mod)) {}

    [[nodiscard]] string to_string() const override
    {
        return "(Attributes[" + access_modifier + "])";
    }
};

class InitNode final : public Node
{
public:
    vector<Token> param_name_toks;
    shared_ptr<Node> body_node;

    InitNode(vector<Token> params, shared_ptr<Node> body, Position ps, Position pe)
        : Node(ps, pe),
          param_name_toks(std::move(params)),
          body_node(std::move(body)) {}

    [[nodiscard]] string to_string() const override
    {
        return "(Init)";
    }
};

class AttrAccessNode final : public Node
{
public:
    shared_ptr<Node> object_node;
    Token attr_name_tok;

    AttrAccessNode(shared_ptr<Node> obj, Token attr_tok)
        : Node(obj->pos_start.value_or(Position()),
               attr_tok.pos_end.value_or(Position())),
          object_node(std::move(obj)),
          attr_name_tok(std::move(attr_tok)) {}

    [[nodiscard]] string to_string() const override
    {
        return "(" + object_node->to_string() + "." + any_cast<string>(attr_name_tok.value) + ")";
    }
};

class AttrAssignNode final : public Node
{
public:
    shared_ptr<Node> object_node;
    Token attr_name_tok;
    shared_ptr<Node> value_node;

    AttrAssignNode(shared_ptr<Node> obj, Token attr_tok, shared_ptr<Node> val)
        : Node(obj->pos_start.value_or(Position()),
               val->pos_end.value_or(Position())),
          object_node(std::move(obj)),
          attr_name_tok(std::move(attr_tok)),
          value_node(std::move(val)) {}

    [[nodiscard]] string to_string() const override
    {
        return "(" + object_node->to_string() + "." +
               any_cast<string>(attr_name_tok.value) + " = " +
               value_node->to_string() + ")";
    }
};
