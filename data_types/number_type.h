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

    explicit Number(long long val) : value(val) {}
    explicit Number(double val) : value(val) {}
    explicit Number(variant<long long, double> val) : value(std::move(val)) {}

    DataType& set_pos(const optional<Position>& start, const optional<Position>& end) override {
        this->pos_start = start;
        this->pos_end = end;
        return *this;
    }

    DataType& set_context(const shared_ptr<Context>& ctx) override {
        this->context = ctx;
        return *this;
    }

    [[nodiscard]] bool is_truthy() const override {
        return std::visit([](auto val) { return val != 0; }, this->value);
    }

    [[nodiscard]] shared_ptr<DataType> copy() const override {
        auto new_num = std::visit([](auto val) { return make_shared<Number>(val); }, this->value);
        new_num->set_pos(this->pos_start, this->pos_end);
        new_num->set_context(this->context);
        return std::static_pointer_cast<DataType>(new_num);
    }

    [[nodiscard]] OperationResult is_true() const override {
        auto result = std::visit([](auto val) { return make_shared<Number>(val); }, this->value);
        result->set_context(this->context);
        result->set_pos(this->pos_start, this->pos_end);
        return std::make_pair(std::static_pointer_cast<DataType>(result), std::nullopt);
    }

    [[nodiscard]] OperationResult add(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            auto new_value = std::visit([](auto left, auto right) -> variant<long long, double> {
                if constexpr (is_integral_v<decltype(left)> && is_integral_v<decltype(right)>) {
                    return left + right;
                }
                return static_cast<double>(left) + static_cast<double>(right);
                }, this->value, other->value);

            auto result = std::visit([](auto val) { return make_shared<Number>(val); }, new_value);
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError({}, {}, "Operand must be a number", this->context));
    }

    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            auto new_value = std::visit([](auto left, auto right) -> variant<long long, double> {
                if constexpr (is_integral_v<decltype(left)> && is_integral_v<decltype(right)>) {
                    return left - right;
                }
                return static_cast<double>(left) - static_cast<double>(right);
                }, this->value, other->value);

            auto result = std::visit([](auto val) { return make_shared<Number>(val); }, new_value);
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError({}, {}, "Operand must be a number", this->context));
    }

    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            auto new_value = std::visit([](auto left, auto right) -> variant<long long, double> {
                if constexpr (is_integral_v<decltype(left)> && is_integral_v<decltype(right)>) {
                    return left * right;
                }
                return static_cast<double>(left) * static_cast<double>(right);
                }, this->value, other->value);

            auto result = std::visit([](auto val) { return make_shared<Number>(val); }, new_value);
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError({}, {}, "Operand must be a number", this->context));
    }

    [[nodiscard]] OperationResult divide(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            if (std::visit([](auto val) { return val == 0; }, other->value)) {
                return std::make_pair(nullptr, RunTimeError(
                    other->pos_start.value_or(Position()),
                    other->pos_end.value_or(Position()),
                    "Division by zero",
                    this->context
                ));
            }
            double new_value = std::visit([](auto left, auto right) {
                return static_cast<double>(left) / static_cast<double>(right);
                }, this->value, other->value);
            auto result = make_shared<Number>(new_value);
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError({}, {}, "Operand must be a number", this->context));
    }

    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            if (std::visit([](auto val) { return val == 0; }, other->value)) {
                return std::make_pair(nullptr, RunTimeError(
                    other->pos_start.value_or(Position()),
                    other->pos_end.value_or(Position()),
                    "Division by zero",
                    this->context
                ));
            }
            auto new_value = std::visit([](auto left, auto right) -> variant<long long, double> {
                if constexpr (is_integral_v<decltype(left)> && is_integral_v<decltype(right)>) {
                    return left % right;
                }
                return fmod(static_cast<double>(left), static_cast<double>(right));
                }, this->value, other->value);

            auto result = std::visit([](auto val) { return make_shared<Number>(val); }, new_value);
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError({}, {}, "Operand must be a number", this->context));
    }

    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            if (std::visit([](auto val) { return val == 0; }, other->value)) {
                return std::make_pair(nullptr, RunTimeError(
                    other->pos_start.value_or(Position()),
                    other->pos_end.value_or(Position()),
                    "Division by zero",
                    this->context
                ));
            }
            auto new_value = std::visit([](auto left, auto right) -> variant<long long, double> {
                if constexpr (is_integral_v<decltype(left)> && is_integral_v<decltype(right)>) {
                    return left / right;
                }
                return floor(static_cast<double>(left) / static_cast<double>(right));
                }, this->value, other->value);

            auto result = std::visit([](auto val) { return make_shared<Number>(val); }, new_value);
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError({}, {}, "Operand must be a number", this->context));
    }

    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            auto new_value = std::visit([](auto left, auto right) -> variant<long long, double> {

                if constexpr (is_integral_v<decltype(left)> && is_integral_v<decltype(right)>) {
                    if (right >= 0) {
                        long long res = 1;
                        long long base = left;
                        long long exp = right;
                        while (exp > 0) {
                            if (exp % 2 == 1) res *= base;
                            base *= base;
                            exp /= 2;
                        }
                        return res;
                    }
                }
                return pow(static_cast<double>(left), static_cast<double>(right));

            }, this->value, other->value);

            const auto result = std::visit([](auto val) { return make_shared<Number>(val); }, new_value);
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError({}, {}, "Operand must be a number", this->context));
    }

    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            const bool result = std::visit([](auto left, auto right) { return left == right; }, this->value, other->value);
            return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(result))), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError({}, {}, "Operand must be a number", this->context));
    }

    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            const bool result = std::visit([](auto left, auto right) { return left != right; }, this->value, other->value);
            return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(result))), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError({}, {}, "Operand must be a number", this->context));
    }

    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            const bool result = std::visit([](auto left, auto right) { return left < right; }, this->value, other->value);
            return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(result))), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError({}, {}, "Operand must be a number", this->context));
    }

    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            const bool result = std::visit([](auto left, auto right) { return left > right; }, this->value, other->value);
            return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(result))), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError({}, {}, "Operand must be a number", this->context));
    }

    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            const bool result = std::visit([](auto left, auto right) { return left <= right; }, this->value, other->value);
            return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(result))), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError({}, {}, "Operand must be a number", this->context));
    }

    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            const bool result = std::visit([](auto left, auto right) { return left >= right; }, this->value, other->value);
            return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(result))), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError({}, {}, "Operand must be a number", this->context));
    }

    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            const bool self_truthy = std::visit([](auto val) { return val != 0; }, this->value);
            const bool other_truthy = std::visit([](auto val) { return val != 0; }, other->value);
            return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(self_truthy && other_truthy))), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError({}, {}, "Operand must be a number", this->context));
    }

    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType>& operand) const override {
        if (const auto other = dynamic_cast<const Number*>(operand.get())) {
            const bool self_truthy = std::visit([](auto val) { return val != 0; }, this->value);
            const bool other_truthy = std::visit([](auto val) { return val != 0; }, other->value);
            return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(self_truthy || other_truthy))), std::nullopt);
        }
        return std::make_pair(nullptr, RunTimeError({}, {}, "Operand must be a number", this->context));
    }

    [[nodiscard]] OperationResult not_by() const override {
        const bool is_truthy = std::visit([](auto val) { return val != 0; }, this->value);
        return std::make_pair(std::static_pointer_cast<DataType>(make_shared<Number>(static_cast<long long>(!is_truthy))), std::nullopt);
    }

    [[nodiscard]] string to_string() const override {
        return std::visit([](auto&& val) { return std::to_string(val); }, this->value);
    }
};