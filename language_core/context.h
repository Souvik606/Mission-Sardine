#pragma once

#include <bits/stdc++.h>
#include "position.h"
#include "symbol_table.h"

using namespace std;

class Context {
public:
    string display_name;
    shared_ptr<Context> parent;
    optional<Position> parent_entry_pos;
    shared_ptr<SymbolTable> symbol_table;

    explicit Context(
        string display_name,
        shared_ptr<Context> parent = nullptr,
        optional<Position> parent_entry_pos = nullopt
    )
        : display_name(std::move(display_name)),
          parent(std::move(parent)),
          parent_entry_pos(std::move(parent_entry_pos)),
          symbol_table(nullptr)
    {
    }
};
