#pragma once

#include <bits/stdc++.h>
#include "../data_types/data_type.h"
#include "../language_core/error.h"

using namespace std;

class RunTimeResult {
public:
    shared_ptr<DataType> value;
    shared_ptr<RunTimeError> error;
    shared_ptr<DataType> func_return_value;
    bool loop_continue;
    bool loop_or_switch_break;

    RunTimeResult() : value(nullptr), error(nullptr), func_return_value(nullptr), loop_continue(false), loop_or_switch_break(false) {
    }

    void reset() {
        this->value = nullptr;
        this->error = nullptr;
        this->func_return_value = nullptr;
        this->loop_continue = false;
        this->loop_or_switch_break = false;
    }

    shared_ptr<DataType> register_result(const RunTimeResult& res) {
        this->error = res.error;
        this->func_return_value = res.func_return_value;
        this->loop_continue = res.loop_continue;
        this->loop_or_switch_break = res.loop_or_switch_break;
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
        this->loop_continue = true;
        return *this;
    }

    RunTimeResult& success_break() {
        this->reset();
        this->loop_or_switch_break = true;
        return *this;
    }

    [[nodiscard]] bool should_return() const {
        return error != nullptr || func_return_value != nullptr || loop_continue || loop_or_switch_break;
    }

    RunTimeResult& failure(const RunTimeError& result_error) {
        this->reset();
        this->error = std::static_pointer_cast<RunTimeError>(result_error.clone());
        return *this;
    }
};