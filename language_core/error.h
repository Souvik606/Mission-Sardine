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
    IllegalCharError(const Position &pos_start, const Position &pos_end, const string &details = "")
        : Error(pos_start, pos_end, "Illegal Character", details) {
    }
};

class InvalidSyntaxError final : public Error {
public:
    InvalidSyntaxError(const Position &pos_start, const Position &pos_end, const string &details = "")
        : Error(pos_start, pos_end, "Invalid Syntax", details) {
    }
};

class RunTimeError final : public Error {
public:
    shared_ptr<Context> context;

    RunTimeError(const Position &pos_start, const Position &pos_end, const string &details, shared_ptr<Context> context)
        : Error(pos_start, pos_end, "Runtime Error", details), context(std::move(context)) {
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
