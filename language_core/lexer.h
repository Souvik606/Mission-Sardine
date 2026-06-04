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

    Token make_fstring() {
        string raw;
        Position pos_start = pos.copy();
        bool escape_character = false;
        advance(); // skip opening '"'

        map<char, char> escape_characters = {
            {'n', '\n'},
            {'t', '\t'}
        };
        int depth = 0; // track nesting of {{ }} so we store them as-is

        while (current_char.has_value() && (current_char.value() != '"' || depth > 0 || escape_character)) {
            if (escape_character) {
                if (depth > 0) {
                    raw += '\\';
                    raw += current_char.value();
                } else {
                    char escaped_char = current_char.value();
                    if (escape_characters.count(escaped_char)) {
                        raw += escape_characters[escaped_char];
                    } else {
                        raw += escaped_char;
                    }
                }
                escape_character = false;
            } else {
                if (current_char.value() == '\\') {
                    escape_character = true;
                } else if (current_char.value() == '{') {
                    depth++;
                    raw += '{';
                } else if (current_char.value() == '}') {
                    if (depth > 0) depth--;
                    raw += '}';
                } else {
                    raw += current_char.value();
                }
            }
            advance();
        }

        advance(); // skip closing '"'
        return Token(T_FSTRING, raw, pos_start, pos);
    }

    shared_ptr<Error> skip_multiline_comment(const Position& pos_start) {
        while (current_char.has_value()) {
            if (current_char.value() == '*') {
                advance();
                if (current_char.has_value() && current_char.value() == '#') {
                    advance();
                    return nullptr;
                }
            } else {
                advance();
            }
        }

        return make_shared<ExpectedCharError>(pos_start, pos, "Closing '*#' for multiline comment");
    }

    shared_ptr<Error> skip_comment() {
        Position pos_start = pos.copy();
        advance();

        if (current_char.has_value() && current_char.value() == '*') {
            advance();
            return skip_multiline_comment(pos_start);
        }

        while (current_char.has_value() && current_char.value() != '\n') {
            advance();
        }
        if (current_char.has_value() && current_char.value() == '\n') {
            advance();
        }

        return nullptr;
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
        if (token_type == T_IDENTIFIER) {
            for (const auto& err_type : ERROR_TYPES) {
                if (id_str == err_type) {
                    token_type = T_ERROR;
                    break;
                }
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

    variant<Token, shared_ptr<Error>> make_not_equals() {
        Position pos_start = pos.copy();
        advance();
        if (current_char.has_value() && current_char.value() == '=') {
            advance();
            return Token(T_NEQ, {}, pos_start, pos);
        }
        advance();
        return make_shared<ExpectedCharError>(pos_start, pos, "'=' (after '!')");
    }

    Token make_lesser() {
        Position pos_start = pos.copy();
        advance();
        string token_type = T_LT;
        if (current_char.has_value() && current_char.value() == '=') {
            advance();
            token_type = T_LTE;
        }
        else if (current_char.has_value() && current_char.value() == '-') {
            advance();
            token_type = T_LARROW;
        }
        else if (current_char.has_value() && current_char.value() == '<') {
            advance();
            token_type = T_LSHIFT;
            if (current_char.has_value() && current_char.value() == '=') {
                advance();
                token_type = T_LSHIFTEQUAL;
            }
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
        else if (current_char.has_value() && current_char.value() == '>') {
            advance();
            token_type = T_RSHIFT;
            if (current_char.has_value() && current_char.value() == '=') {
                advance();
                token_type = T_RSHIFTEQUAL;
            }
        }
        return Token(token_type, {}, pos_start, pos);
    }

    Token make_bitand() {
        Position pos_start = pos.copy();
        advance();
        string token_type = T_BITAND;
        if (current_char.has_value() && current_char.value() == '=') {
            advance();
            token_type = T_BITANDEQUAL;
        }
        return Token(token_type, {}, pos_start, pos);
    }

    Token make_bitxor() {
        Position pos_start = pos.copy();
        advance();
        string token_type = T_BITXOR;
        if (current_char.has_value() && current_char.value() == '=') {
            advance();
            token_type = T_BITXOREQUAL;
        }
        return Token(token_type, {}, pos_start, pos);
    }

    Token make_bitor() {
        Position pos_start = pos.copy();
        advance();
        string token_type = T_BITOR;
        if (current_char.has_value() && current_char.value() == '=') {
            advance();
            token_type = T_BITOREQUAL;
        }
        return Token(token_type, {}, pos_start, pos);
    }

    Token make_plus() {
        Position pos_start = pos.copy();
        advance();
        string token_type = T_PLUS;
        if (current_char.has_value() && current_char.value() == '=') {
            advance();
            token_type = T_PLUSEQUAL;
        }
        return Token(token_type, {}, pos_start, pos);
    }

    Token make_sub() {
        Position pos_start = pos.copy();
        advance();
        string token_type = T_MINUS;
        if (current_char.has_value() && current_char.value() == '>') {
            advance();
            token_type = T_ARROW;
        }
        else if (current_char.has_value() && current_char.value() == '=') {
            advance();
            token_type = T_MINUSEQUAL;
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
            if (current_char.has_value() && current_char.value() == '=') {
                advance();
                token_type = T_EXPEQUAL;
            }
        }
        else if (current_char.has_value() && current_char.value() == '=') {
            advance();
            token_type = T_MULEQUAL;
        }
        return Token(token_type, {}, pos_start, pos);
    }

    Token make_div() {
        Position pos_start = pos.copy();
        advance();
        string token_type = T_DIVIDE;
        if (current_char.has_value() && current_char.value() == '/') {
            advance();
            token_type = T_FLOOR;
            if (current_char.has_value() && current_char.value() == '=') {
                advance();
                token_type = T_FLOOREQUAL;
            }
        }
        else if (current_char.has_value() && current_char.value() == '=') {
            advance();
            token_type = T_DIVIDEEQUAL;
        }
        return Token(token_type, {}, pos_start, pos);
    }

    Token make_mod() {
        Position pos_start = pos.copy();
        advance();
        string token_type = T_MODULUS;
        if (current_char.has_value() && current_char.value() == '=') {
            advance();
            token_type = T_MODULUSEQUAL;
        }
        return Token(token_type, {}, pos_start, pos);
    }

    variant<Token, shared_ptr<Error>> make_number() {
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
        try {
            if (is_float) {
                return Token(T_FLOAT, stod(number_str), pos_start, pos);
            }
            else {
                try {
                    return Token(T_INT, stoll(number_str), pos_start, pos);
                } catch (...) {
                    return Token(T_INT, stod(number_str), pos_start, pos);
                }
            }
        } catch (...) {
            return make_shared<IllegalCharError>(pos_start, pos, "Numerical literal exceeds digit conversion limits or is invalid");
        }
    }

    pair<vector<Token>, shared_ptr<Error>> enumerate_tokens() {
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
                auto result = make_number();
                if (holds_alternative<shared_ptr<Error>>(result)) {
                    return { {}, get<shared_ptr<Error>>(result) };
                }
                tokens.push_back(get<Token>(result));
            }
            else if (LETTERS.find(c) != string::npos || c == '_') {
                tokens.push_back(make_identifier());
            }
            else if (c == '"') {
                tokens.push_back(make_string());
            }
            else if (c == '$') {
                Position pos_start = pos.copy();
                advance();
                if (current_char.has_value() && current_char.value() == '"') {
                    tokens.push_back(make_fstring());
                } else {
                    return { {}, make_shared<IllegalCharError>(pos_start, pos, "\"$\"") };
                }
            }
            else if (c == '#') {
                auto err = skip_comment();
                if (err) return { {}, err };
            }
            else if (c == '+') {
                tokens.push_back(make_plus());
            }
            else if (c == '-') {
                tokens.push_back(make_sub());
            }
            else if (c == '*') {
                tokens.push_back(make_mul());
            }
            else if (c == '/') {
                tokens.push_back(make_div());
            }
            else if (c == '%') {
                tokens.push_back(make_mod());
            }
            else if (c == '=') {
                tokens.push_back(make_equals());
            }
            else if (c == '!') {
                auto result = make_not_equals();
                if (std::holds_alternative<shared_ptr<Error>>(result)) {
                    return { {}, std::get<shared_ptr<Error>>(result) };
                }
                tokens.push_back(std::get<Token>(result));
            }
            else if (c == '<') {
                tokens.push_back(make_lesser());
            }
            else if (c == '>') {
                tokens.push_back(make_greater());
            }
            else if (c == '&') {
                tokens.push_back(make_bitand());
            }
            else if (c == '^') {
                tokens.push_back(make_bitxor());
            }
            else if (c == '|') {
                tokens.push_back(make_bitor());
            }
            else if (c == '~') {
                tokens.push_back(Token(T_BITNOT, {}, pos));
                advance();
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
            else if (c == '.') {
                tokens.push_back(Token(T_DOT, {}, pos));
                advance();
            }
            else {
                const Position pos_start = pos.copy();
                const char illegal_char = c;
                advance();
                string details = "\"";
                details += illegal_char;
                details += "\"";
                return { {}, make_shared<IllegalCharError>(pos_start, pos, details) };
            }
        }
        tokens.push_back(Token(T_EOF, {}, pos));
        return { tokens, nullptr };
    }
};