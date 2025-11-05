#pragma once
#include <bits/stdc++.h>
#include "../language_core/position.h"

class RunTimeError;
using namespace std;

class DataType;
class Number;
class Context;

class DataType {
public:
    optional<Position> pos_start;
    optional<Position> pos_end;
    shared_ptr<Context> context;

    virtual ~DataType() = default;

    virtual DataType& set_pos(const optional<Position>& start, const optional<Position>& end) {
        this->pos_start = start;
        this->pos_end = end;
        return *this;
    }

    DataType& set_pos() {
        return set_pos({}, {});
    }

    virtual DataType& set_context(const shared_ptr<Context>& ctx) {
        this->context = ctx;
        return *this;
    }

    DataType& set_context() {
        return set_context({});
    }

    using OperationResult = pair<shared_ptr<Number>, optional<RunTimeError>>;

    [[nodiscard]] virtual shared_ptr<DataType> copy() const = 0;
    [[nodiscard]] virtual string to_string() const = 0;
    [[nodiscard]] virtual OperationResult add(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult subtract(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult multiply(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult divide(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult get_comparison_eq(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult get_comparison_neq(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult get_comparison_lt(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult get_comparison_gt(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult get_comparison_lte(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult get_comparison_gte(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult and_by(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult or_by(const shared_ptr<DataType>& other) const = 0;
    [[nodiscard]] virtual OperationResult not_by() const = 0;
};