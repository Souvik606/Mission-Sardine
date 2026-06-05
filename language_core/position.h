#pragma once
#include <bits/stdc++.h>
using namespace std;

class Position {
public:
    int index = -1;
    int line = 0;
    int col = -1;
    shared_ptr<const string> file_name = nullptr;
    shared_ptr<const string> file_text = nullptr;

    Position() = default;

    Position(const int index, const int line, const int col, shared_ptr<const string> file_name, shared_ptr<const string> file_text) {
        this->index = index;
        this->line = line;
        this->col = col;
        this->file_name = std::move(file_name);
        this->file_text = std::move(file_text);
    }

    Position(const int index, const int line, const int col, const string &file_name, const string &file_text) {
        this->index = index;
        this->line = line;
        this->col = col;
        this->file_name = make_shared<const string>(file_name);
        this->file_text = make_shared<const string>(file_text);
    }

    Position advance(const char current_char = '\0') {
        index += 1;
        col += 1;
        if (current_char == '\n') {
            line += 1;
            col = 0;
        }
        return *this;
    }

    [[nodiscard]] Position copy() const {
        return {index, line, col, file_name, file_text};
    }
};
