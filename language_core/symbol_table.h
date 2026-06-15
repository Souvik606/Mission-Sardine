#pragma once
#include <bits/stdc++.h>
#include "string_interner.h"

using namespace std;
class DataType;

class SymbolTable {
public:
    shared_ptr<SymbolTable> parent;
    uint64_t id;

private:
    unordered_map<const string*, shared_ptr<DataType>> symbols;

public:
    explicit SymbolTable(shared_ptr<SymbolTable> p = nullptr) : parent(std::move(p)) {
        static uint64_t next_id = 1;
        id = next_id++;
    }

    [[nodiscard]] shared_ptr<DataType> get(const string* interned_name) const {
        if (const auto it = symbols.find(interned_name); it != symbols.end()) {
            return it->second;
        }

        if (parent) {
            return parent->get(interned_name);
        }

        return nullptr;
    }

    [[nodiscard]] shared_ptr<DataType> get(const string& name) const {
        return get(StringInterner::intern(name));
    }

    [[nodiscard]] const shared_ptr<DataType>* get_ptr(const string* interned_name, const SymbolTable*& found_table) const {
        auto it = symbols.find(interned_name);
        if (it != symbols.end()) {
            found_table = this;
            return &it->second;
        }
        if (parent) {
            return parent->get_ptr(interned_name, found_table);
        }
        found_table = nullptr;
        return nullptr;
    }

    [[nodiscard]] const shared_ptr<DataType>* get_ptr(const string& name, const SymbolTable*& found_table) const {
        return get_ptr(StringInterner::intern(name), found_table);
    }

    void reset(shared_ptr<SymbolTable> p = nullptr) {
        symbols.clear();
        parent = std::move(p);
        static uint64_t next_id = 1;
        id = next_id++;
    }

    void set(const string* interned_name, shared_ptr<DataType> value) {
        symbols[interned_name] = std::move(value);
    }

    void set(const string& name, shared_ptr<DataType> value) {
        set(StringInterner::intern(name), std::move(value));
    }

    void remove(const string* interned_name) {
        symbols.erase(interned_name);
    }

    void remove(const string& name) {
        remove(StringInterner::intern(name));
    }

    [[nodiscard]] const unordered_map<const string*, shared_ptr<DataType>>& get_symbols() const {
        return symbols;
    }
};