#pragma once
#include <bits/stdc++.h>
#include <memory>
#include "context.h"
#include "position.h"
using namespace std;

class Error {
public:
    virtual ~Error() = default;

    Position pos_start;
    Position pos_end;
    string error_name;
    string details;

    Error(Position pos_start, Position pos_end, string error_name, string details)
        : pos_start(std::move(pos_start)),
        pos_end(std::move(pos_end)),
        error_name(std::move(error_name)),
        details(std::move(details)) {
    }

    [[nodiscard]] virtual string to_string() const {
        string rel_path = get_relative_path(pos_start.file_name);
        string snippet = string_with_arrows(pos_start.file_text, pos_start, pos_end);
        stringstream ss;
        ss << "  File " << rel_path << ", line " << (pos_start.line + 1) << "\n"
           << snippet << "\n"
           << error_name << ": " << details;
        return ss.str();
    }

    [[nodiscard]] virtual shared_ptr<Error> clone() const {
        return make_shared<Error>(*this);
    }

protected:
    static string get_relative_path(const string& file_path) {
        if (file_path.empty()) return "";
        try {
            namespace fs = std::filesystem;
            fs::path p(file_path);
            if (p.is_absolute()) {
                auto rel = fs::relative(p, fs::current_path());
                string r_str = rel.generic_string();
                return r_str;
            }
            string r_str = file_path;
            for (char& c : r_str) {
                if (c == '\\') c = '/';
            }
            return r_str;
        } catch (...) {
            string r_str = file_path;
            for (char& c : r_str) {
                if (c == '\\') c = '/';
            }
            return r_str;
        }
    }

    static string string_with_arrows(const string& text, const Position& pos_start, const Position& pos_end) {
        vector<string> lines;
        stringstream ss(text);
        string item;
        while (getline(ss, item, '\n')) {
            lines.push_back(item);
        }
        if (lines.empty()) return "";

        int idx = max(0, min(pos_start.line, (int)lines.size() - 1));
        string line = lines[idx];

        int col_start = max(0, min(pos_start.col, (int)line.size()));
        int col_end;
        if (pos_end.line == pos_start.line) {
            col_end = max(col_start + 1, min(pos_end.col + 1, (int)line.size()));
        } else {
            col_end = max(col_start + 1, (int)line.size());
        }

        string indent = "    ";
        string snippet = indent + line + "\n";
        snippet += indent + string(col_start, ' ') + string(col_end - col_start, '^');
        return snippet;
    }
};

class IllegalCharError final : public Error {
public:
    IllegalCharError(const Position& pos_start, const Position& pos_end, const string& details = "")
        : Error(pos_start, pos_end, "Illegal Character", details) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<IllegalCharError>(*this);
    }
};

class ExpectedCharError final : public Error {
public:
    ExpectedCharError(const Position& pos_start, const Position& pos_end, const string& details = "")
        : Error(pos_start, pos_end, "Expected Character", details) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<ExpectedCharError>(*this);
    }
};

class InvalidSyntaxError final : public Error {
public:
    InvalidSyntaxError(const Position& pos_start, const Position& pos_end, const string& details = "")
        : Error(pos_start, pos_end, "Invalid Syntax", details) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<InvalidSyntaxError>(*this);
    }
};

class RunTimeError : public Error {
public:
    shared_ptr<Context> context;

    RunTimeError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, const string& error_name = "RunTimeError")
        : Error(pos_start, pos_end, error_name, details), context(std::move(context)) {
    }

    [[nodiscard]] string to_string() const override {
        return generate_traceback();
    }

    [[nodiscard]] string generate_traceback() const {
        vector<string> frames;
        Position pos = this->pos_start;
        shared_ptr<Context> ctx = this->context;

        while (ctx) {
            string rel_path = get_relative_path(pos.file_name);
            Position end_pos = (ctx->parent == nullptr) ? this->pos_end : pos;
            string snippet = string_with_arrows(pos.file_text, pos, end_pos);

            stringstream frame_ss;
            frame_ss << "  File " << rel_path << ", line " << (pos.line + 1)
                     << ", in " << ctx->display_name << "\n"
                     << snippet;
            frames.push_back(frame_ss.str());

            pos = ctx->parent_entry_pos.value_or(Position());
            ctx = ctx->parent;
        }

        vector<string> reversed_frames = frames;
        reverse(reversed_frames.begin(), reversed_frames.end());

        stringstream traceback_body;
        for (size_t i = 0; i < reversed_frames.size(); ++i) {
            traceback_body << reversed_frames[i];
            if (i + 1 < reversed_frames.size()) {
                traceback_body << "\n";
            }
        }

        stringstream final_ss;
        final_ss << "Traceback (most recent call last):\n"
                 << traceback_body.str() << "\n"
                 << this->error_name << ": " << this->details;

        return final_ss.str();
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<RunTimeError>(*this);
    }
};

class IllegalOperationError final : public RunTimeError {
public:
    IllegalOperationError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "IllegalOperationError") {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<IllegalOperationError>(*this);
    }
};

class DivisionByZeroError final : public RunTimeError {
public:
    DivisionByZeroError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "DivisionByZeroError") {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<DivisionByZeroError>(*this);
    }
};

class IndexOutOfBoundsError final : public RunTimeError {
public:
    IndexOutOfBoundsError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "IndexOutOfBoundsError") {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<IndexOutOfBoundsError>(*this);
    }
};

class NameError final : public RunTimeError {
public:
    NameError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "NameError") {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<NameError>(*this);
    }
};

class ArgumentError final : public RunTimeError {
public:
    ArgumentError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "ArgumentError") {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<ArgumentError>(*this);
    }
};

class NotImplementedError final : public RunTimeError {
public:
    NotImplementedError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "NotImplementedError") {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<NotImplementedError>(*this);
    }
};

class InvalidErrorTypeError final : public RunTimeError {
public:
    InvalidErrorTypeError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "InvalidErrorTypeError") {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<InvalidErrorTypeError>(*this);
    }
};

class DictKeyError final : public RunTimeError {
public:
    DictKeyError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "DictKeyError") {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<DictKeyError>(*this);
    }
};

class AttributeError final : public RunTimeError {
public:
    AttributeError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "AttributeError") {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<AttributeError>(*this);
    }
};

class TypeError final : public RunTimeError {
public:
    TypeError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "TypeError") {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<TypeError>(*this);
    }
};