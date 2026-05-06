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
        stringstream ss;
        ss << error_name << ": " << details
            << "\nFile " << pos_start.file_name
            << ", line " << (pos_start.line + 1);
        return ss.str();
    }
};

class IllegalCharError final : public Error {
public:
    IllegalCharError(const Position& pos_start, const Position& pos_end, const string& details = "")
        : Error(pos_start, pos_end, "Illegal Character", details) {
    }
};

class ExpectedCharError final : public Error {
public:
    ExpectedCharError(const Position& pos_start, const Position& pos_end, const string& details = "")
        : Error(pos_start, pos_end, "Expected Character", details) {
    }
};

class InvalidSyntaxError final : public Error {
public:
    InvalidSyntaxError(const Position& pos_start, const Position& pos_end, const string& details = "")
        : Error(pos_start, pos_end, "Invalid Syntax", details) {
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
        stringstream traceback;
        Position pos = this->pos_start;
        shared_ptr<Context> ctx = this->context;

        while (ctx) {
            traceback << "  File " << pos.file_name
                << ", line " << (pos.line + 1)
                << ", in " << ctx->display_name << "\n";

            pos = ctx->parent_entry_pos.value_or(Position());
            ctx = ctx->parent;
        }

        stringstream final_ss;
        final_ss << "Traceback (most recent call last):\n"
            << traceback.str()
            << this->error_name << ": " << this->details;

        return final_ss.str();
    }
};

class IllegalOperationError final : public RunTimeError {
public:
    IllegalOperationError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "IllegalOperationError") {
    }
};

class DivisionByZeroError final : public RunTimeError {
public:
    DivisionByZeroError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "DivisionByZeroError") {
    }
};

class IndexOutOfBoundsError final : public RunTimeError {
public:
    IndexOutOfBoundsError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "IndexOutOfBoundsError") {
    }
};

class NameError final : public RunTimeError {
public:
    NameError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "NameError") {
    }
};

class ArgumentError final : public RunTimeError {
public:
    ArgumentError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "ArgumentError") {
    }
};

class NotImplementedError final : public RunTimeError {
public:
    NotImplementedError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "NotImplementedError") {
    }
};

class InvalidErrorTypeError final : public RunTimeError {
public:
    InvalidErrorTypeError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "InvalidErrorTypeError") {
    }
};

class DictKeyError final : public RunTimeError {
public:
    DictKeyError(const Position& pos_start, const Position& pos_end, const string& details, shared_ptr<Context> context)
        : RunTimeError(pos_start, pos_end, details, context, "DictKeyError") {
    }
};