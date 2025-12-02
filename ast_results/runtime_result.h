#pragma once

#include <bits/stdc++.h>
#include "../data_types/data_type.h"
#include "../language_core/error.h"

using namespace std;

class RunTimeResult {
public:
    shared_ptr<DataType> value;
    optional<RunTimeError> error;
    shared_ptr<DataType> func_return_value;
    bool loop_continue;
    bool loop_break;

    RunTimeResult() {
        reset();
    }

    void reset() {
        this->value = nullptr;
        this->error = nullopt;
        this->func_return_value = nullptr;
        this->loop_continue = false;
        this->loop_break = false;
    }

    shared_ptr<DataType> register_result(const RunTimeResult &res) {
        this->error = res.error;
        this->func_return_value = res.func_return_value;
        this->loop_continue = res.loop_continue;
        this->loop_break = res.loop_break;
        return res.value;
    }

    RunTimeResult &success(shared_ptr<DataType> result_value) {
        reset();
        this->value = std::move(result_value);
        return *this;
    }

    RunTimeResult &success_return(shared_ptr<DataType> result_value) {
        reset();
        this->func_return_value = std::move(result_value);
        return *this;
    }

    RunTimeResult &success_continue() {
        reset();
        this->loop_continue = true;
        return *this;
    }

    RunTimeResult &success_break() {
        reset();
        this->loop_break = true;
        return *this;
    }

    RunTimeResult &failure(const RunTimeError &result_error) {
        reset();
        this->error = result_error;
        return *this;
    }

    bool should_return() const {
        return error.has_value() || func_return_value != nullptr || loop_continue || loop_break;
    }
};