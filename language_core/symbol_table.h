#pragma once
#include <bits/stdc++.h>
#include "../data_types/data_type.h"

using namespace std;

class SymbolTable {
public:
    shared_ptr<SymbolTable> parent;

private:
    unordered_map<string, shared_ptr<DataType>> symbols;

public:
    SymbolTable() : parent(nullptr) {}

    [[nodiscard]] shared_ptr<DataType> get(const string& name) const {
        if (const auto it = symbols.find(name); it != symbols.end()) {
            return it->second;
        }

        if (parent) {
            return parent->get(name);
        }

        return nullptr;
    }

    void set(const string& name, shared_ptr<DataType> value) {
        symbols[name] = std::move(value);
    }

    void remove(const string& name) {
        symbols.erase(name);
    }
};