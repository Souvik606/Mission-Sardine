#pragma once

#include <bits/stdc++.h>
#include "../data_types/data_type.h"
#include "../language_core/error.h"

using namespace std;

class RunTimeResult {
public:
    shared_ptr<DataType> value;
    optional<RunTimeError> error;

    RunTimeResult() : value(nullptr), error(nullopt) {
    }

    shared_ptr<DataType> register_result(const RunTimeResult &res) {
        if (res.error) {
            this->error = res.error;
        }
        return res.value;
    }

    RunTimeResult &success(shared_ptr<DataType> result_value) {
        this->value = std::move(result_value);
        return *this;
    }

    RunTimeResult &failure(const RunTimeError &result_error) {
        this->error = result_error;
        return *this;
    }
};
