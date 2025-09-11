#pragma once
#include <bits/stdc++.h>
#include "data_type.h"
#include "../language_core/error.h"
#include "../language_core/context.h"
#include "../language_core/position.h"

using namespace std;

class Number final : public DataType {
public:
    variant<long long, double> value;

    optional<Position> pos_start;
    optional<Position> pos_end;
    shared_ptr<Context> context;

    explicit Number(long long val) : value(val), pos_start(nullopt), pos_end(nullopt), context(nullptr) {
    }

    explicit Number(double val) : value(val), pos_start(nullopt), pos_end(nullopt), context(nullptr) {
    }

    Number &set_pos(const optional<Position> &start = nullopt, const optional<Position> &end = nullopt) {
        this->pos_start = start;
        this->pos_end = end;
        return *this;
    }

    Number &set_context(const shared_ptr<Context> &ctx = nullptr) {
        this->context = ctx;
        return *this;
    }

    using NumberResult = pair<shared_ptr<Number>, optional<RunTimeError> >;

    [[nodiscard]] NumberResult add(const shared_ptr<DataType> &operand) const {
        if (const auto other = dynamic_cast<Number *>(operand.get())) {
            auto new_value = std::visit([](auto left, auto right) -> variant<long long, double> {
                if constexpr (is_integral_v<decltype(left)> && is_integral_v<decltype(right)>) {
                    return left + right; // int + int -> int
                }
                return static_cast<double>(left) + static_cast<double>(right); // Otherwise -> double
            }, this->value, other->value);

            auto result = std::visit([](auto val) { return make_shared<Number>(val); }, new_value);
            result->set_context(this->context);
            return {result, nullopt};
        }
        return {nullptr, RunTimeError({}, {}, "Operand must be a number", this->context)};
    }

    [[nodiscard]] NumberResult subtract(const shared_ptr<DataType> &operand) const {
        if (const auto other = dynamic_cast<Number *>(operand.get())) {
            auto new_value = std::visit([](auto left, auto right) -> variant<long long, double> {
                if constexpr (is_integral_v<decltype(left)> && is_integral_v<decltype(right)>) {
                    return left - right; // int - int -> int
                }
                return static_cast<double>(left) - static_cast<double>(right); // Otherwise -> double
            }, this->value, other->value);

            auto result = std::visit([](auto val) { return make_shared<Number>(val); }, new_value);
            result->set_context(this->context);
            return {result, nullopt};
        }
        return {nullptr, RunTimeError({}, {}, "Operand must be a number", this->context)};
    }

    [[nodiscard]] NumberResult multiply(const shared_ptr<DataType> &operand) const {
        if (const auto other = dynamic_cast<Number *>(operand.get())) {
            auto new_value = std::visit([](auto left, auto right) -> variant<long long, double> {
                if constexpr (is_integral_v<decltype(left)> && is_integral_v<decltype(right)>) {
                    return left * right; // int * int -> int
                }
                return static_cast<double>(left) * static_cast<double>(right); // Otherwise -> double
            }, this->value, other->value);

            auto result = std::visit([](auto val) { return make_shared<Number>(val); }, new_value);
            result->set_context(this->context);
            return {result, nullopt};
        }
        return {nullptr, RunTimeError({}, {}, "Operand must be a number", this->context)};
    }

    [[nodiscard]] NumberResult divide(const shared_ptr<DataType> &operand) const {
        if (const auto other = dynamic_cast<Number *>(operand.get())) {
            if (std::visit([](auto val) { return val == 0; }, other->value)) {
                return {
                    nullptr, RunTimeError(
                        other->pos_start.value_or(Position()),
                        other->pos_end.value_or(Position()),
                        "Division by zero",
                        this->context
                    )
                };
            }

            double new_value = std::visit([](auto left, auto right) {
                return static_cast<double>(left) / static_cast<double>(right);
            }, this->value, other->value);

            auto result = make_shared<Number>(new_value);
            result->set_context(this->context);
            return {result, nullopt};
        }
        return {nullptr, RunTimeError({}, {}, "Operand must be a number", this->context)};
    }

    [[nodiscard]] string to_string() const override {
        return std::visit([](auto &&val) { return std::to_string(val); }, this->value);
    }
};
