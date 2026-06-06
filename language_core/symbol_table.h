#pragma once
#include <bits/stdc++.h>

using namespace std;
class DataType;

class SymbolTable {
public:
    shared_ptr<SymbolTable> parent;
    uint64_t id;

private:
    unordered_map<string, shared_ptr<DataType>> symbols;

public:
    explicit SymbolTable(shared_ptr<SymbolTable> p = nullptr) : parent(std::move(p)) {
        static uint64_t next_id = 1;
        id = next_id++;
    }

    [[nodiscard]] shared_ptr<DataType> get(const string& name) const {
        if (const auto it = symbols.find(name); it != symbols.end()) {
            return it->second;
        }

        if (parent) {
            return parent->get(name);
        }

        return nullptr;
    }

    [[nodiscard]] const shared_ptr<DataType>* get_ptr(const string& name, const SymbolTable*& found_table) const {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            found_table = this;
            return &it->second;
        }
        if (parent) {
            return parent->get_ptr(name, found_table);
        }
        found_table = nullptr;
        return nullptr;
    }

    void reset(shared_ptr<SymbolTable> p = nullptr) {
        symbols.clear();
        parent = std::move(p);
        static uint64_t next_id = 1;
        id = next_id++;
    }

    void set(const string& name, shared_ptr<DataType> value) {
        symbols[name] = std::move(value);
    }

    void remove(const string& name) {
        symbols.erase(name);
    }

    [[nodiscard]] const unordered_map<string, shared_ptr<DataType>>& get_symbols() const {
        return symbols;
    }
};