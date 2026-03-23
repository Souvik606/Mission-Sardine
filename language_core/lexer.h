#pragma once

#include <bits/stdc++.h>

#include "error.h"
#include "position.h"
#include "constants.h"

using namespace std;

class Token {
public:
    string type;
    any value;
    optional<Position> pos_start;
    optional<Position> pos_end;

    explicit Token(string type, any value = {}, optional<Position> pos_start = nullopt,
        optional<Position> pos_end = nullopt) {
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
            if (value.type() == typeid(long long)) {
                ss << any_cast<long long>(value);
            }
            else if (value.type() == typeid(double)) {
                ss << any_cast<double>(value);
            }
            else if (value.type() == typeid(string)) {
                ss << "\"" << any_cast<string>(value) << "\"";
            }
            else if (value.type() == typeid(const char*)) {
                ss << "\"" << any_cast<const char*>(value) << "\"";
            }
            else {
                ss << "[unprintable value]";
            }
        }
        else {
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
        }
        else {
            current_char = nullopt;
        }
    }

    Token make_string() {
        string str;
        Position pos_start = pos.copy();
        bool escape_character = false;
        advance();

        map<char, char> escape_characters = {
            {'n', '\n'},
            {'t', '\t'}
        };

        while (current_char.has_value() && (current_char.value() != '"' || escape_character)) {
            if (escape_character) {
                char escaped_char = current_char.value();
                if (escape_characters.count(escaped_char)) {
                    str += escape_characters[escaped_char];
                }
                else {
                    str += escaped_char;
                }
                escape_character = false;
            }
            else {
                if (current_char.value() == '\\') {
                    escape_character = true;
                }
                else {
                    str += current_char.value();
                }
            }
            advance();
        }

        advance();
        return Token(T_STRING, str, pos_start, pos);
    }

    Token make_identifier() {
        string id_str;
        Position pos_start = pos.copy();
        const string valid_chars = LETTERS_DIGITS + "_";
        while (current_char.has_value() && valid_chars.find(current_char.value()) != string::npos) {
            id_str += current_char.value();
            advance();
        }
        string token_type = T_IDENTIFIER;
        for (const auto& keyword : KEYWORDS) {
            if (id_str == keyword) {
                token_type = T_KEYWORD;
                break;
            }
        }
        return Token(token_type, id_str, pos_start, pos);
    }

    Token make_equals() {
        Position pos_start = pos.copy();
        advance();
        string token_type = T_EQ;
        if (current_char.has_value() && current_char.value() == '=') {
            advance();
            token_type = T_EE;
        }
        return Token(token_type, {}, pos_start, pos);
    }

    variant<Token, Error> make_not_equals() {
        Position pos_start = pos.copy();
        advance();
        if (current_char.has_value() && current_char.value() == '=') {
            advance();
            return Token(T_NEQ, {}, pos_start, pos);
        }
        advance();
        return ExpectedCharError(pos_start, pos, "'=' (after '!')");
    }

    Token make_lesser() {
        Position pos_start = pos.copy();
        advance();
        string token_type = T_LT;
        if (current_char.has_value() && current_char.value() == '=') {
            advance();
            token_type = T_LTE;
        }
        return Token(token_type, {}, pos_start, pos);
    }

    Token make_greater() {
        Position pos_start = pos.copy();
        advance();
        string token_type = T_GT;
        if (current_char.has_value() && current_char.value() == '=') {
            advance();
            token_type = T_GTE;
        }
        return Token(token_type, {}, pos_start, pos);
    }

    Token make_mul() {
        Position pos_start = pos.copy();
        advance();
        string token_type = T_MUL;
        if (current_char.has_value() && current_char.value() == '*') {
            advance();
            token_type = T_EXP;
        }
        return Token(token_type, {}, pos_start, pos);
    }

    Token make_number() {
        string number_str;
        bool is_float = false;
        Position pos_start = pos.copy();
        const string valid_chars = DIGITS + ".";
        while (current_char.has_value() && valid_chars.find(current_char.value()) != string::npos) {
            if (current_char.value() == '.') {
                if (is_float) break;
                is_float = true;
                number_str += '.';
            }
            else {
                number_str += current_char.value();
            }
            advance();
        }
        if (is_float) {
            return Token(T_FLOAT, stod(number_str), pos_start, pos);
        }
        else {
            return Token(T_INT, stoll(number_str), pos_start, pos);
        }
    }

    pair<vector<Token>, optional<Error>> enumerate_tokens() {
        vector<Token> tokens;
        while (current_char.has_value()) {
            char c = current_char.value();
            if (c == ' ' || c == '\t') {
                advance();
            }
            else if (c == ';') {
                tokens.push_back(Token(T_NEWLINE, {}, pos));
                advance();
            }
            else if (c == '\n') {
                tokens.push_back(Token(T_NEWLINE, {}, pos));
                advance();
            }
            else if (DIGITS.find(c) != string::npos) {
                tokens.push_back(make_number());
            }
            else if (LETTERS.find(c) != string::npos) {
                tokens.push_back(make_identifier());
            }
            else if (c == '"') {
                tokens.push_back(make_string());
            }
            else if (c == '+') {
                tokens.push_back(Token(T_PLUS, {}, pos));
                advance();
            }
            else if (c == '-') {
                tokens.push_back(Token(T_MINUS, {}, pos));
                advance();
            }
            else if (c == '*') {
                tokens.push_back(make_mul());
            }
            else if (c == '/') {
                tokens.push_back(Token(T_DIVIDE, {}, pos));
                advance();
            }
            else if (c == '=') {
                tokens.push_back(make_equals());
            }
            else if (c == '!') {
                auto result = make_not_equals();
                if (std::holds_alternative<Error>(result)) {
                    return { {}, std::get<Error>(result) };
                }
                tokens.push_back(std::get<Token>(result));
            }
            else if (c == '<') {
                tokens.push_back(make_lesser());
            }
            else if (c == '>') {
                tokens.push_back(make_greater());
            }
            else if (c == '(') {
                tokens.push_back(Token(T_LPAREN, {}, pos));
                advance();
            }
            else if (c == ')') {
                tokens.push_back(Token(T_RPAREN, {}, pos));
                advance();
            }
            else if (c == '{') {
                tokens.push_back(Token(T_LPAREN2, {}, pos));
                advance();
            }
            else if (c == '}') {
                tokens.push_back(Token(T_RPAREN2, {}, pos));
                advance();
            }
            else if (c == '[') {
                tokens.push_back(Token(T_LPAREN3, {}, pos));
                advance();
            }
            else if (c == ']') {
                tokens.push_back(Token(T_RPAREN3, {}, pos));
                advance();
            }
            else if (c == ':') {
                tokens.push_back(Token(T_COLON, {}, pos));
                advance();
            }
            else if (c == '?') {
                tokens.push_back(Token(T_QUESTION, {}, pos));
                advance();
            }
            else if (c == ',') {
                tokens.push_back(Token(T_COMMA, {}, pos));
                advance();
            }
            else {
                const Position pos_start = pos.copy();
                const char illegal_char = c;
                advance();
                string details = "\"";
                details += illegal_char;
                details += "\"";
                return { {}, IllegalCharError(pos_start, pos, details) };
            }
        }
        tokens.push_back(Token(T_EOF, {}, pos));
        return { tokens, nullopt };
    }
};