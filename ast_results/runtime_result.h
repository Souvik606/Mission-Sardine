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
    bool loop_should_continue;
    bool loop_should_break;

    RunTimeResult() : value(nullptr), error(nullopt), func_return_value(nullptr), loop_should_continue(false), loop_should_break(false) {
    }

    void reset() {
        this->value = nullptr;
        this->error = nullopt;
        this->func_return_value = nullptr;
        this->loop_should_continue = false;
        this->loop_should_break = false;
    }

    shared_ptr<DataType> register_result(const RunTimeResult& res) {
        this->error = res.error;
        this->func_return_value = res.func_return_value;
        this->loop_should_continue = res.loop_should_continue;
        this->loop_should_break = res.loop_should_break;
        return res.value;
    }

    RunTimeResult& success(shared_ptr<DataType> result_value) {
        this->reset();
        this->value = std::move(result_value);
        return *this;
    }

    RunTimeResult& success_return(shared_ptr<DataType> result_value) {
        this->reset();
        this->func_return_value = std::move(result_value);
        return *this;
    }

    RunTimeResult& success_continue() {
        this->reset();
        this->loop_should_continue = true;
        return *this;
    }

    RunTimeResult& success_break() {
        this->reset();
        this->loop_should_break = true;
        return *this;
    }

    [[nodiscard]] bool should_return() const {
        return (bool)error || (bool)func_return_value || loop_should_continue || loop_should_break;
    }

    RunTimeResult& failure(const RunTimeError& result_error) {
        this->reset();
        this->error = result_error;
        return *this;
    }
};
