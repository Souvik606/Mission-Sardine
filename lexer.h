#pragma once

#include <bits/stdc++.h>
#include "error.h"

using namespace std;

const string DIGITS = "0123456789";
const string T_INT = "INT";
const string T_FLOAT = "FLOAT";
const string T_PLUS = "PLUS";
const string T_MINUS = "MINUS";
const string T_MUL = "MUL";
const string T_DIVIDE = "DIVIDE";
const string T_LPAREN = "LPAREN";
const string T_RPAREN = "RPAREN";
const string T_EOF = "EOF";

class Token {
public:
    string type;
    any value;
    optional<Position> pos_start;
    optional<Position> pos_end;

    explicit Token(string type, any value = {}, optional<Position> pos_start = nullopt, optional<Position> pos_end = nullopt) {
        this->type = std::move(type);
        this->value = std::move(value);

        if (pos_start) {
            this->pos_start = pos_start->copy();
            this->pos_end = pos_start->copy();
            this->pos_end->advance();
        }

        if (pos_end) {
            this->pos_end = *pos_end;
        }
    }

    [[nodiscard]] string to_string() const {
        stringstream ss;
        if (value.has_value()) {
            ss << type << ": ";
            if (value.type() == typeid(int)) {
                ss << any_cast<int>(value);
            } else if (value.type() == typeid(double)) {
                ss << any_cast<double>(value);
            } else if (value.type() == typeid(string)) {
                ss << any_cast<string>(value);
            } else if (value.type() == typeid(const char*)) {
                ss << any_cast<const char*>(value);
            } else {
                ss << "[unprintable value]";
            }
        } else {
            ss << type;
        }
        return ss.str();
    }
};

class Lexer {
public:
    string filename;
    string text;
    Position pos;
    optional<char> current_char;

    Lexer(const string& filename, const string& text)
        : filename(filename),
          text(text),
          pos(Position(-1, 0, -1, filename, text)),
          current_char(nullopt) {
        advance();
    }

    void advance() {
        pos.advance(current_char.value_or('\0'));
        if (pos.index < text.length()) {
            current_char = text[pos.index];
        } else {
            current_char = nullopt;
        }
    }

    Token make_number() {
        string number_str;
        bool is_float = false;
        Position pos_start = pos.copy();

        string valid_chars = DIGITS + ".";
        while (current_char.has_value() && valid_chars.find(current_char.value()) != string::npos) {
            if (current_char.value() == '.') {
                if (is_float) {
                    break;
                }
                is_float = true;
                number_str += '.';
            } else {
                number_str += current_char.value();
            }
            advance();
        }

        if (is_float) {
            return Token(T_FLOAT, stod(number_str), pos_start, pos);
        } else {
            return Token(T_INT, stoi(number_str), pos_start, pos);
        }
    }

    pair<vector<Token>, optional<Error>> enumerate_tokens() {
        vector<Token> tokens;

        while (current_char.has_value()) {
            char c = current_char.value();

            if (c == ' ' || c == '\t') {
                advance();
            } else if (DIGITS.find(c) != string::npos) {
                tokens.push_back(make_number());
            } else if (c == '+') {
                tokens.push_back(Token(T_PLUS, {}, pos));
                advance();
            } else if (c == '-') {
                tokens.push_back(Token(T_MINUS, {}, pos));
                advance();
            } else if (c == '*') {
                tokens.push_back(Token(T_MUL, {}, pos));
                advance();
            } else if (c == '/') {
                tokens.push_back(Token(T_DIVIDE, {}, pos));
                advance();
            } else if (c == '(') {
                tokens.push_back(Token(T_LPAREN, {}, pos));
                advance();
            } else if (c == ')') {
                tokens.push_back(Token(T_RPAREN, {}, pos));
                advance();
            } else {
                Position pos_start = pos.copy();
                char illegal_char = c;
                advance();
                string details = "\"";
                details += illegal_char;
                details += "\"";
                return {{}, IllegalCharError(pos_start, pos, details)};
            }
        }

        tokens.push_back(Token(T_EOF, {}, pos));
        return {tokens, nullopt};
    }
};