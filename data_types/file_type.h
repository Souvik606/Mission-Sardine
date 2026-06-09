#pragma once
#include <bits/stdc++.h>
#include <cstdio>
#include "data_type.h"
#include "number_type.h"
#include "string_type.h"
#include "null_type.h"
#include "../language_core/error.h"
#include "../language_core/constants.h"

using namespace std;

struct FileDescriptor {
    FILE* fp = nullptr;
    bool closed = false;

    explicit FileDescriptor(FILE* f) : fp(f) {}

    ~FileDescriptor() {
        if (fp && !closed) {
            fclose(fp);
        }
    }

    bool close() {
        if (fp && !closed) {
            closed = true;
            int rc = fclose(fp);
            fp = nullptr;
            return rc == 0;
        }
        return true;
    }

    // Read entire remaining content
    string read_all() const {
        if (!fp || closed) return "";
        string result;
        char buf[4096];
        while (fgets(buf, sizeof(buf), fp)) {
            if (!UNBOUNDED_MODE && result.length() + strlen(buf) > 10000000) {
                throw std::length_error("File read size limit exceeded (max 10MB)");
            }
            result += buf;
        }
        return result;
    }

    // Read one line
    string read_line() const {
        if (!fp || closed) return "";
        char buf[4096];
        if (fgets(buf, sizeof(buf), fp)) {
            return string(buf);
        }
        return "";
    }

    // Write a string
    bool write(const string& text) const {
        if (!fp || closed) return false;
        return fputs(text.c_str(), fp) != EOF;
    }

    bool is_closed() const { return closed || fp == nullptr; }
};

// ── Sards File DataType ───────────────────────────────────────────────────────
class File final : public DataType {
public:
    string filepath;
    string mode;
    shared_ptr<FileDescriptor> descriptor;

    File(string filepath, string mode, shared_ptr<FileDescriptor> desc)
        : filepath(std::move(filepath)), mode(std::move(mode)), descriptor(std::move(desc)) {}

    [[nodiscard]] string get_type_name() const override { return "File"; }

    [[nodiscard]] bool is_file() const override { return true; }

    // Truthiness: open file is truthy, closed is falsy (matches Python: not Null -> True)
    [[nodiscard]] bool is_truthy() const override {
        return descriptor && !descriptor->is_closed();
    }

    [[nodiscard]] OperationResult is_true() const override {
        bool open = descriptor && !descriptor->is_closed();
        auto result = Number::make_bool(open);
        result->set_context(this->context);
        result->set_pos(this->pos_start, this->pos_end);
        return {result, nullptr};
    }

    // copy() shares the same underlying FileDescriptor (shallow copy as per migration tips)
    [[nodiscard]] shared_ptr<DataType> copy() const override {
        auto c = make_shared<File>(filepath, mode, descriptor);
        c->set_pos(pos_start, pos_end);
        c->set_context(context);
        return c;
    }

    [[nodiscard]] string to_string() const override {
        bool closed = !descriptor || descriptor->is_closed();
        string state = closed ? "closed" : "open";
        return "<file '" + filepath + "' mode='" + mode + "' state=" + state + ">";
    }

    // Reference equality (identity comparison) matching migration tips §2
    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType>& other) const override {
        // Compare against Null -> always false
        if (other->get_type_name() == "Null") {
            return {Number::make_bool(false), nullptr};
        }
        // Identity: same underlying descriptor pointer
        if (auto other_file = dynamic_pointer_cast<File>(other)) {
            bool same = (descriptor.get() == other_file->descriptor.get());
            return {Number::make_bool(same), nullptr};
        }
        return {Number::make_bool(false), nullptr};
    }

    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType>& other) const override {
        auto [eq_res, eq_err] = get_comparison_eq(other);
        if (eq_err) return {nullptr, eq_err};
        auto num = dynamic_pointer_cast<Number>(eq_res);
        bool val = num ? !num->is_truthy() : true;
        return {Number::make_bool(val), nullptr};
    }

    // ── Attribute access: bound methods ──────────────────────────────────────
    [[nodiscard]] OperationResult get_attr(const string& attr_name, const shared_ptr<Context>& calling_context) const override {
        // Helper to build an error position
        auto ps = pos_start.value_or(Position());
        auto pe = pos_end.value_or(Position());

        // ── read() ───────────────────────────────────────────────────────────
        if (attr_name == "read") {
            auto self_copy = std::static_pointer_cast<File>(copy());
            auto impl = [](const shared_ptr<DataType>& self_dt,
                           const vector<shared_ptr<DataType>>& pos_args,
                           const map<string, shared_ptr<DataType>>& kw_args,
                           const shared_ptr<Context>& exec_ctx) -> OperationResult
            {
                auto self = dynamic_pointer_cast<File>(self_dt);
                auto ps = self->pos_start.value_or(Position());
                auto pe = self->pos_end.value_or(Position());
                if (!pos_args.empty() || !kw_args.empty()) {
                    return {nullptr, make_shared<ArgumentError>(ps, pe, "read() takes no arguments", exec_ctx)};
                }
                if (!self->descriptor || self->descriptor->is_closed()) {
                    return {nullptr, make_shared<FileIOError>(ps, pe, "I/O operation on closed file.", exec_ctx)};
                }
                if (self->mode != "r" && self->mode != "r+") {
                    return {nullptr, make_shared<FileIOError>(ps, pe, "File not open for reading", exec_ctx)};
                }
                try {
                    string content = self->descriptor->read_all();
                    auto str = make_shared<String>(content);
                    str->set_context(exec_ctx);
                    return {str, nullptr};
                } catch (const exception& e) {
                    return {nullptr, make_shared<FileIOError>(ps, pe, string("Failed to read file: ") + e.what(), exec_ctx)};
                }
            };
            auto bound = make_shared<BoundMethod>("read", self_copy, impl);
            bound->set_context(calling_context).set_pos(pos_start, pos_end);
            return {bound, nullptr};
        }

        // ── lread() ──────────────────────────────────────────────────────────
        if (attr_name == "lread") {
            auto self_copy = std::static_pointer_cast<File>(copy());
            auto impl = [](const shared_ptr<DataType>& self_dt,
                           const vector<shared_ptr<DataType>>& pos_args,
                           const map<string, shared_ptr<DataType>>& kw_args,
                           const shared_ptr<Context>& exec_ctx) -> OperationResult
            {
                auto self = dynamic_pointer_cast<File>(self_dt);
                auto ps = self->pos_start.value_or(Position());
                auto pe = self->pos_end.value_or(Position());
                if (!pos_args.empty() || !kw_args.empty()) {
                    return {nullptr, make_shared<ArgumentError>(ps, pe, "lread() takes no arguments", exec_ctx)};
                }
                if (!self->descriptor || self->descriptor->is_closed()) {
                    return {nullptr, make_shared<FileIOError>(ps, pe, "I/O operation on closed file.", exec_ctx)};
                }
                if (self->mode != "r" && self->mode != "r+") {
                    return {nullptr, make_shared<FileIOError>(ps, pe, "File not open for reading", exec_ctx)};
                }
                try {
                    string line = self->descriptor->read_line();
                    auto str = make_shared<String>(line);
                    str->set_context(exec_ctx);
                    return {str, nullptr};
                } catch (const exception& e) {
                    return {nullptr, make_shared<FileIOError>(ps, pe, string("Failed to read line: ") + e.what(), exec_ctx)};
                }
            };
            auto bound = make_shared<BoundMethod>("lread", self_copy, impl);
            bound->set_context(calling_context).set_pos(pos_start, pos_end);
            return {bound, nullptr};
        }

        // ── write() ──────────────────────────────────────────────────────────
        if (attr_name == "write") {
            auto self_copy = std::static_pointer_cast<File>(copy());
            auto impl = [](const shared_ptr<DataType>& self_dt,
                           const vector<shared_ptr<DataType>>& pos_args,
                           const map<string, shared_ptr<DataType>>& kw_args,
                           const shared_ptr<Context>& exec_ctx) -> OperationResult
            {
                auto self = dynamic_pointer_cast<File>(self_dt);
                auto ps = self->pos_start.value_or(Position());
                auto pe = self->pos_end.value_or(Position());
                if (pos_args.size() != 1 || !kw_args.empty()) {
                    return {nullptr, make_shared<ArgumentError>(ps, pe, "write() takes exactly 1 argument: (text)", exec_ctx)};
                }
                if (!self->descriptor || self->descriptor->is_closed()) {
                    return {nullptr, make_shared<FileIOError>(ps, pe, "I/O operation on closed file.", exec_ctx)};
                }
                const string& m = self->mode;
                if (m != "w" && m != "a" && m != "r+" && m != "w+" && m != "a+") {
                    return {nullptr, make_shared<FileIOError>(ps, pe, "File not open for writing", exec_ctx)};
                }
                auto text_arg = dynamic_pointer_cast<String>(pos_args[0]);
                if (!text_arg) {
                    auto arg_ps = pos_args[0]->pos_start.value_or(Position());
                    auto arg_pe = pos_args[0]->pos_end.value_or(Position());
                    return {nullptr, make_shared<TypeError>(arg_ps, arg_pe, "write() argument must be a String", exec_ctx)};
                }
                try {
                    self->descriptor->write(text_arg->value);
                    return {make_shared<Null>(), nullptr};
                } catch (const exception& e) {
                    return {nullptr, make_shared<FileIOError>(ps, pe, string("Failed to write to file: ") + e.what(), exec_ctx)};
                }
            };
            auto bound = make_shared<BoundMethod>("write", self_copy, impl);
            bound->set_context(calling_context).set_pos(pos_start, pos_end);
            return {bound, nullptr};
        }

        // ── lwrite() ─────────────────────────────────────────────────────────
        if (attr_name == "lwrite") {
            auto self_copy = std::static_pointer_cast<File>(copy());
            auto impl = [](const shared_ptr<DataType>& self_dt,
                           const vector<shared_ptr<DataType>>& pos_args,
                           const map<string, shared_ptr<DataType>>& kw_args,
                           const shared_ptr<Context>& exec_ctx) -> OperationResult
            {
                auto self = dynamic_pointer_cast<File>(self_dt);
                auto ps = self->pos_start.value_or(Position());
                auto pe = self->pos_end.value_or(Position());
                if (pos_args.size() != 1 || !kw_args.empty()) {
                    return {nullptr, make_shared<ArgumentError>(ps, pe, "lwrite() takes exactly 1 argument: (text)", exec_ctx)};
                }
                if (!self->descriptor || self->descriptor->is_closed()) {
                    return {nullptr, make_shared<FileIOError>(ps, pe, "I/O operation on closed file.", exec_ctx)};
                }
                const string& m = self->mode;
                if (m != "w" && m != "a" && m != "r+" && m != "w+" && m != "a+") {
                    return {nullptr, make_shared<FileIOError>(ps, pe, "File not open for writing", exec_ctx)};
                }
                auto text_arg = dynamic_pointer_cast<String>(pos_args[0]);
                if (!text_arg) {
                    auto arg_ps = pos_args[0]->pos_start.value_or(Position());
                    auto arg_pe = pos_args[0]->pos_end.value_or(Position());
                    return {nullptr, make_shared<TypeError>(arg_ps, arg_pe, "lwrite() argument must be a String", exec_ctx)};
                }
                try {
                    self->descriptor->write(text_arg->value + "\n");
                    return {make_shared<Null>(), nullptr};
                } catch (const exception& e) {
                    return {nullptr, make_shared<FileIOError>(ps, pe, string("Failed to write line to file: ") + e.what(), exec_ctx)};
                }
            };
            auto bound = make_shared<BoundMethod>("lwrite", self_copy, impl);
            bound->set_context(calling_context).set_pos(pos_start, pos_end);
            return {bound, nullptr};
        }

        // ── close() ──────────────────────────────────────────────────────────
        if (attr_name == "close") {
            auto self_copy = std::static_pointer_cast<File>(copy());
            auto impl = [](const shared_ptr<DataType>& self_dt,
                           const vector<shared_ptr<DataType>>& pos_args,
                           const map<string, shared_ptr<DataType>>& kw_args,
                           const shared_ptr<Context>& exec_ctx) -> OperationResult
            {
                auto self = dynamic_pointer_cast<File>(self_dt);
                auto ps = self->pos_start.value_or(Position());
                auto pe = self->pos_end.value_or(Position());
                if (!pos_args.empty() || !kw_args.empty()) {
                    return {nullptr, make_shared<ArgumentError>(ps, pe, "close() takes no arguments", exec_ctx)};
                }
                try {
                    if (self->descriptor) {
                        self->descriptor->close();
                    }
                    return {make_shared<Null>(), nullptr};
                } catch (const exception& e) {
                    return {nullptr, make_shared<FileIOError>(ps, pe, string("Failed to close file: ") + e.what(), exec_ctx)};
                }
            };
            auto bound = make_shared<BoundMethod>("close", self_copy, impl);
            bound->set_context(calling_context).set_pos(pos_start, pos_end);
            return {bound, nullptr};
        }

        // Unknown attribute
        return {nullptr, make_shared<AttributeError>(
            ps, pe,
            "'File' object has no attribute '" + attr_name + "'",
            calling_context
        )};
    }

    // ── Read all lines from the file (used by foreach / comprehension) ────────
    // Returns lines as strings; throws on I/O errors. Caller checks closed state.
    vector<string> read_lines() const {
        vector<string> lines;
        if (!descriptor || descriptor->is_closed() || !descriptor->fp) return lines;
        char buf[4096];
        size_t total_bytes = 0;
        while (fgets(buf, sizeof(buf), descriptor->fp)) {
            size_t len = strlen(buf);
            total_bytes += len;
            if (!UNBOUNDED_MODE && total_bytes > 10000000) {
                throw std::length_error("File read size limit exceeded (max 10MB)");
            }
            lines.emplace_back(buf);
        }
        return lines;
    }

    // ── Unsupported arithmetic/logic operators ────────────────────────────────
    [[nodiscard]] OperationResult add(const shared_ptr<DataType>& o) const override { return err("'+'"); }
    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType>& o) const override { return err("'-'"); }
    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType>& o) const override { return err("'*'"); }
    [[nodiscard]] OperationResult divide(const shared_ptr<DataType>& o) const override { return err("'/'"); }
    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType>& o) const override { return err("'%'"); }
    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType>& o) const override { return err("'**'"); }
    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType>& o) const override { return err("'//'"); }
    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType>& o) const override { return err("'<'"); }
    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType>& o) const override { return err("'>'"); }
    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType>& o) const override { return err("'<='"); }
    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType>& o) const override { return err("'>='"); }
    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType>& o) const override { return err("'and'"); }
    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType>& o) const override { return err("'or'"); }
    [[nodiscard]] OperationResult not_by() const override { return err("'not'"); }
    [[nodiscard]] OperationResult bitwise_and(const shared_ptr<DataType>& o) const override { return err("'&'"); }
    [[nodiscard]] OperationResult bitwise_xor(const shared_ptr<DataType>& o) const override { return err("'^'"); }
    [[nodiscard]] OperationResult bitwise_or(const shared_ptr<DataType>& o) const override { return err("'|'"); }
    [[nodiscard]] OperationResult bitwise_not() const override { return err("'~'"); }
    [[nodiscard]] OperationResult lshift(const shared_ptr<DataType>& o) const override { return err("'<<'"); }
    [[nodiscard]] OperationResult rshift(const shared_ptr<DataType>& o) const override { return err("'>>'"); }

private:
    [[nodiscard]] OperationResult err(const string& op) const {
        return {nullptr, make_shared<IllegalOperationError>(
            pos_start.value_or(Position()), pos_end.value_or(Position()),
            "Cannot apply " + op + " to a File", context
        )};
    }
};
