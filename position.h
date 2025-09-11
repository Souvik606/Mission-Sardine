#pragma once
#include <bits/stdc++.h>
using namespace std;

class Position {
public:
    int index = -1;
    int line = 0;
    int col = -1;
    string file_name;
    string file_text;

    Position() = default;

    Position(const int index, const int line, const int col, const string &file_name, const string &file_text) {
        this->index = index;
        this->line = line;
        this->col = col;
        this->file_name = file_name;
        this->file_text = file_text;
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