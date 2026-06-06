#pragma once
#include <bits/stdc++.h>
#include <memory>
#include "context.h"
#include "position.h"
using namespace std;

class CleanExitException : public std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "CleanExitException";
    }
};


class Error {
public:
    virtual ~Error() = default;

    Position pos_start;
    Position pos_end;
    string error_name;
    string details;
    string error_code;
    string hint;

    Error(Position pos_start, Position pos_end, string error_name, string details, string error_code = "E0000", string hint = "")
        : pos_start(std::move(pos_start)),
        pos_end(std::move(pos_end)),
        error_name(std::move(error_name)),
        details(std::move(details)),
        error_code(std::move(error_code)),
        hint(std::move(hint)) {
    }

    [[nodiscard]] virtual string to_string() const {
        string rel_path = pos_start.file_name ? get_relative_path(*pos_start.file_name) : "";
        string snippet = pos_start.file_text ? string_with_arrows(*pos_start.file_text, pos_start, pos_end) : "";
        stringstream ss;
        ss << "  [" << error_code << "] File " << (rel_path.empty() ? "<unknown>" : rel_path) << ", line " << (pos_start.line + 1) << "\n"
           << snippet << "\n"
           << error_name << ": " << details;
        if (!hint.empty()) {
            ss << "\n  Hint: " << hint;
        }
        return ss.str();
    }

    [[nodiscard]] virtual shared_ptr<Error> clone() const {
        return make_shared<Error>(*this);
    }

protected:
    static string get_relative_path(const string& file_path) {
        if (file_path.empty()) return "";
        string f_path = file_path;
        for (char& c : f_path) {
            if (c == '\\') c = '/';
        }
        if (f_path.length() >= 2 && f_path[1] == ':') {
            f_path[0] = toupper(f_path[0]);
        }
        try {
            namespace fs = std::filesystem;
            string cur_path = fs::current_path().generic_string();
            for (char& c : cur_path) {
                if (c == '\\') c = '/';
            }
            if (cur_path.length() >= 2 && cur_path[1] == ':') {
                cur_path[0] = toupper(cur_path[0]);
            }
            if (!cur_path.empty() && cur_path.back() != '/') {
                cur_path += '/';
            }
            if (f_path.length() >= cur_path.length()) {
                bool is_prefix = true;
                for (size_t i = 0; i < cur_path.length(); ++i) {
                    if (toupper(f_path[i]) != toupper(cur_path[i])) {
                        is_prefix = false;
                        break;
                    }
                }
                if (is_prefix) {
                    string r_str = f_path.substr(cur_path.length());
                    if (r_str.rfind("stdlib/", 0) == 0) {
                        r_str = "sards/" + r_str;
                    }
                    return r_str;
                }
            }
        } catch (...) {}
        if (f_path.rfind("stdlib/", 0) == 0) {
            f_path = "sards/" + f_path;
        }
        return f_path;
    }

    static string string_with_arrows(const string& text, const Position& pos_start, const Position& pos_end) {
        if (text.empty()) return "";
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
            col_end = max(col_start + 1, min(pos_end.col, (int)line.size()));
        } else {
            col_end = max(col_start + 1, (int)line.size());
        }

        string indent = "    ";

        // Smart Truncation for exceptionally long lines (e.g. minified files or flat expressions)
        const int MAX_LINE_LENGTH = 120;
        if ((int)line.size() > MAX_LINE_LENGTH) {
            int error_width = col_end - col_start;
            int remaining = MAX_LINE_LENGTH - error_width;
            int left_padding = max(0, remaining / 2);

            int start_idx = max(0, col_start - left_padding);
            int end_idx = min((int)line.size(), start_idx + MAX_LINE_LENGTH);

            // Re-adjust start_idx if near the end of the line to show more left context
            if (end_idx - start_idx < MAX_LINE_LENGTH) {
                start_idx = max(0, end_idx - MAX_LINE_LENGTH);
            }

            string truncated = line.substr(start_idx, end_idx - start_idx);

            int offset = -start_idx;
            if (start_idx > 0) {
                truncated = "..." + truncated;
                offset += 3;
            }
            if (end_idx < (int)line.size()) {
                truncated += "...";
            }

            int new_col_start = max(0, min((int)truncated.size(), col_start + offset));
            int new_col_end = max(new_col_start + 1, min((int)truncated.size(), col_end + offset));

            string snippet = indent + truncated + "\n";
            snippet += indent + string(new_col_start, ' ') + string(new_col_end - new_col_start, '^');
            return snippet;
        }

        string snippet = indent + line + "\n";
        snippet += indent + string(col_start, ' ') + string(col_end - col_start, '^');
        return snippet;
    }
};

class IllegalCharError final : public Error {
public:
    IllegalCharError(const Position& pos_start, const Position& pos_end, const string& details = "", string hint = "")
        : Error(pos_start, pos_end, "Illegal Character Error", details, "E1001", hint.empty() ? "Remove or replace the unrecognised character." : std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<IllegalCharError>(*this);
    }
};

class ExpectedCharError final : public Error {
public:
    ExpectedCharError(const Position& pos_start, const Position& pos_end, const string& details = "", string hint = "")
        : Error(pos_start, pos_end, "Expected Character", details, "E1002", std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<ExpectedCharError>(*this);
    }
};

class InvalidSyntaxError final : public Error {
public:
    InvalidSyntaxError(const Position& pos_start, const Position& pos_end, const string& details = "", string hint = "")
        : Error(pos_start, pos_end, "Invalid Syntax Error", details, "E2001", std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<InvalidSyntaxError>(*this);
    }
};

class RunTimeError : public Error {
public:
    shared_ptr<Context> context;

    RunTimeError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, const string& error_name = "RunTimeError", string error_code = "E9001", string hint = "")
        : Error(pos_start, pos_end, error_name, details, std::move(error_code), std::move(hint)), context(std::move(context)) {
    }

    [[nodiscard]] string to_string() const override {
        return generate_traceback();
    }

    [[nodiscard]] string generate_traceback() const {
        vector<string> frames;
        Position pos = this->pos_start;
        shared_ptr<Context> ctx = this->context;
        unordered_set<uintptr_t> visited;

        while (ctx && frames.size() < 1000) {
            if (visited.count(reinterpret_cast<uintptr_t>(ctx.get()))) {
                break;
            }
            visited.insert(reinterpret_cast<uintptr_t>(ctx.get()));

            string rel_path;
            string snippet;
            string line_str;
            if (!pos.file_name || pos.file_name->empty()) {
                rel_path = "<unknown>";
                snippet = "";
                line_str = "?";
            } else {
                rel_path = get_relative_path(*pos.file_name);
                Position end_pos = (ctx->parent == nullptr) ? this->pos_end : pos;
                snippet = pos.file_text ? string_with_arrows(*pos.file_text, pos, end_pos) : "";
                line_str = std::to_string(pos.line + 1);
            }

            stringstream frame_ss;
            frame_ss << "  File " << rel_path << ", line " << line_str
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
        final_ss << "[" << error_code << "] Traceback (most recent call last):\n"
                 << traceback_body.str() << "\n"
                 << this->error_name << ": " << this->details;
        if (!hint.empty()) {
            final_ss << "\n  Hint: " << hint;
        }

        return final_ss.str();
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<RunTimeError>(*this);
    }
};

class IllegalOperationError final : public RunTimeError {
public:
    IllegalOperationError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, string hint = "")
        : RunTimeError(pos_start, pos_end, details, std::move(context), "IllegalOperationError", "E5001", std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<IllegalOperationError>(*this);
    }
};

class DivisionByZeroError final : public RunTimeError {
public:
    DivisionByZeroError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, string hint = "")
        : RunTimeError(pos_start, pos_end, details, std::move(context), "DivisionByZeroError", "E5002",
                       hint.empty() ? "Check that the divisor is not zero before dividing. Example: `when b != 0 { result = a / b }`" : std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<DivisionByZeroError>(*this);
    }
};

class IndexOutOfBoundsError final : public RunTimeError {
public:
    IndexOutOfBoundsError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, string hint = "")
        : RunTimeError(pos_start, pos_end, details, std::move(context), "IndexOutOfBoundsError", "E7001",
                       hint.empty() ? "Use `len(collection) - 1` to find the last valid index, or check bounds before accessing." : std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<IndexOutOfBoundsError>(*this);
    }
};

class NameError final : public RunTimeError {
public:
    NameError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, string hint = "")
        : RunTimeError(pos_start, pos_end, details, std::move(context), "NameError", "E3001",
                       hint.empty() ? "Check for typos or make sure the variable is assigned before use." : std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<NameError>(*this);
    }
};

class ArgumentError final : public RunTimeError {
public:
    ArgumentError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, string hint = "")
        : RunTimeError(pos_start, pos_end, details, std::move(context), "ArgumentError", "E6001", std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<ArgumentError>(*this);
    }
};

class NotImplementedError final : public RunTimeError {
public:
    NotImplementedError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, string hint = "")
        : RunTimeError(pos_start, pos_end, details, std::move(context), "NotImplementedError", "E9004", std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<NotImplementedError>(*this);
    }
};

class InvalidErrorTypeError final : public RunTimeError {
public:
    InvalidErrorTypeError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, string hint = "")
        : RunTimeError(pos_start, pos_end, details, std::move(context), "InvalidErrorTypeError", "E9003",
                       hint.empty() ? "Valid error types are: RunTimeError, IllegalOperationError, DivisionByZeroError, IndexOutOfBoundsError, NameError, ArgumentError, TypeError, AttributeError, DictKeyError, ValueError, ModuleError, StackDepthExceededError." : std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<InvalidErrorTypeError>(*this);
    }
};

class DictKeyError final : public RunTimeError {
public:
    DictKeyError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, string hint = "")
        : RunTimeError(pos_start, pos_end, details, std::move(context), "DictKeyError", "E7003",
                       hint.empty() ? "Check whether the key exists before accessing it." : std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<DictKeyError>(*this);
    }
};

class AttributeError final : public RunTimeError {
public:
    AttributeError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, string hint = "")
        : RunTimeError(pos_start, pos_end, details, std::move(context), "AttributeError", "E7002", std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<AttributeError>(*this);
    }
};

class TypeError final : public RunTimeError {
public:
    TypeError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, string hint = "")
        : RunTimeError(pos_start, pos_end, details, std::move(context), "TypeError", "E4001", std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<TypeError>(*this);
    }
};

class ValueError final : public RunTimeError {
public:
    ValueError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, string hint = "")
        : RunTimeError(pos_start, pos_end, details, std::move(context), "ValueError", "E9005", std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<ValueError>(*this);
    }
};

class ModuleError final : public RunTimeError {
public:
    ModuleError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, string hint = "")
        : RunTimeError(pos_start, pos_end, details, std::move(context), "ModuleError", "E8001", std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<ModuleError>(*this);
    }
};

class StackDepthExceededError final : public RunTimeError {
public:
    StackDepthExceededError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, string hint = "")
        : RunTimeError(pos_start, pos_end, details, std::move(context), "StackDepthExceededError", "E9002",
                       hint.empty() ? "This usually means a function is calling itself infinitely. Check your recursive functions for a proper base case." : std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<StackDepthExceededError>(*this);
    }
};

class FileIOError final : public RunTimeError {
public:
    FileIOError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context, string hint = "")
        : RunTimeError(pos_start, pos_end, details, std::move(context), "FileIOError", "E9006",
                       hint.empty() ? "Verify that the path is correct, the file exists, and you have necessary permissions." : std::move(hint)) {
    }

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<FileIOError>(*this);
    }
};

class DataType;

class UserDefinedError final : public RunTimeError {
public:
    shared_ptr<DataType> instance;

    UserDefinedError(const Position& pos_start, const Position& pos_end, shared_ptr<DataType> inst, shared_ptr<Context> context, string hint = "");

    [[nodiscard]] shared_ptr<Error> clone() const override {
        return make_shared<UserDefinedError>(*this);
    }

private:
    static string get_name_from_instance(const shared_ptr<DataType>& inst);
    static string get_details_from_instance(const shared_ptr<DataType>& inst);
};