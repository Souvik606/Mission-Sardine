#include <bits/stdc++.h>
using namespace std;

class Position {
public:
    int index;
    int line;
    int col;
    string file_name;
    string file_text;

    Position(int index, int line, int col, string file_name, string file_text) {
        this->index = index;
        this->line = line;
        this->col = col;
        this->file_name = file_name;
        this->file_text = file_text;
    }

    Position advance(char current_char = '\0') {
        index += 1;
        col += 1;
        if (current_char == '\n') {
            line += 1;
            col = 0;
        }
        return *this;
    }

    Position copy() {
        return Position(index, line, col, file_name, file_text);
    }
};

class Error {
public:
    Position pos_start;
    Position pos_end;
    string error_name;
    string details;

    Error(Position pos_start, Position pos_end, string error_name, string details)
        : pos_start(pos_start), pos_end(pos_end), error_name(error_name), details(details) {}

    string to_string() {
        stringstream ss;
        ss << error_name << ": " << details
           << "\nFile " << pos_start.file_name
           << ", line " << (pos_start.line + 1);
        return ss.str();
    }
};

class IllegalCharError : public Error {
public:
    IllegalCharError(Position pos_start, Position pos_end, string details = "")
        : Error(pos_start, pos_end, "Illegal Character", details) {}
};

class InvalidSyntaxError : public Error {
public:
    InvalidSyntaxError(Position pos_start, Position pos_end, string details = "")
        : Error(pos_start, pos_end, "Invalid Syntax", details) {}
};