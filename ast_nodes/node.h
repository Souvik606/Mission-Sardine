#pragma once

#include <bits/stdc++.h>
#include "../language_core/position.h"

using namespace std;

class DataType;

enum NodeType {
    NODE_NUMBER = 0,
    NODE_STRING,
    NODE_LIST,
    NODE_BINARY_OPERATION,
    NODE_TERNARY_OPERATION,
    NODE_UNARY_OPERATION,
    NODE_VARIABLE_USE,
    NODE_VARIABLE_ASSIGN,
    NODE_IF,
    NODE_SWITCH,
    NODE_FOR,
    NODE_WHILE,
    NODE_FUNCTION_DEFINITION,
    NODE_FUNCTION_CALL,
    NODE_RETURN,
    NODE_CONTINUE,
    NODE_BREAK,
    NODE_DICT,
    NODE_TRY,
    NODE_MODEL,
    NODE_ATTR_ACCESS,
    NODE_ATTR_ASSIGN,
    NODE_FSTRING,
    NODE_FOREACH_LOOP,
    NODE_SUMMON,
    NODE_LIST_COMPREHENSION,
    NODE_DICT_COMPREHENSION,
    NODE_INDEX_ACCESS,
    NODE_UNKNOWN = -1
};

class Node {
public:
    optional<Position> pos_start;
    optional<Position> pos_end;
    int depth = 1;
    mutable int node_type_id = -1;
    mutable shared_ptr<DataType> cached_value = nullptr;

    [[nodiscard]] virtual int get_node_type() const { return NODE_UNKNOWN; }

    Node(optional<Position> start, optional<Position> end)
        : pos_start(std::move(start)), pos_end(std::move(end)) {
    }

    virtual ~Node() = default;

    [[nodiscard]] virtual std::string to_string() const { return ""; }
};
