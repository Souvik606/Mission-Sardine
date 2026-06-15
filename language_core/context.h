#pragma once

#include <bits/stdc++.h>
#include "position.h"
#include "symbol_table.h"

using namespace std;

class ModelType;

class Context {
public:
    string display_name;
    shared_ptr<Context> parent;
    optional<Position> parent_entry_pos;
    shared_ptr<SymbolTable> symbol_table;
    shared_ptr<ModelType> owner_class;
    int depth;

    explicit Context(
        string display_name,
        shared_ptr<Context> parent = nullptr,
        optional<Position> parent_entry_pos = nullopt
    )
        : display_name(std::move(display_name)),
          parent(parent),
          parent_entry_pos(std::move(parent_entry_pos)),
          symbol_table(nullptr),
          owner_class(nullptr),
          depth(parent ? parent->depth + 1 : 1)
    {
    }

    void reset(string new_display_name, shared_ptr<Context> new_parent, optional<Position> new_parent_entry_pos) {
        display_name = std::move(new_display_name);
        parent = std::move(new_parent);
        parent_entry_pos = std::move(new_parent_entry_pos);
        owner_class = nullptr;
        depth = parent ? parent->depth + 1 : 1;
    }
};
