#pragma once
#include <bits/stdc++.h>
#include "data_type.h"
#include "../language_core/error.h"
#include "../language_core/context.h"
#include "../language_core/position.h"

using namespace std;

class Number final : public DataType, public enable_shared_from_this<Number> {
public:
    variant<long long, double> value;
    bool is_float = false;
    bool is_boolean = false;

    static double round_to_13_dec(double val) {
        return std::round(val * 1e13) / 1e13;
    }

    explicit Number(long long val, bool is_flt = false, bool is_bool = false) : value(val), is_float(is_flt), is_boolean(is_bool) {}
    explicit Number(double val, bool is_flt = true) : value(val), is_float(is_flt), is_boolean(false) {}
    explicit Number(variant<long long, double> val) : value(std::move(val)) {
        is_float = holds_alternative<double>(value);
        is_boolean = false;
    }
    Number(variant<long long, double> val, bool is_flt, bool is_bool) : value(std::move(val)), is_float(is_flt), is_boolean(is_bool) {}

    [[nodiscard]] string get_type_name() const override {
        if (is_boolean) return "Boolean";
        return is_float ? "Float" : "Integer";
    }
    // ── Small-integer cache ───────────────────────────────────────────────────
    static constexpr long long CACHE_MIN = -1;
    static constexpr long long CACHE_MAX = 256;

    static shared_ptr<Number> make_bool(bool b) {
        static auto true_val = shared_ptr<Number>(new Number(1LL, false, true));
        static auto false_val = shared_ptr<Number>(new Number(0LL, false, true));
        return b ? true_val : false_val;
    }

    static shared_ptr<Number> make(long long v, bool is_flt = false, bool is_bool = false) {
        if (is_bool) {
            return make_bool(v != 0);
        }
        if (!is_flt && v >= CACHE_MIN && v <= CACHE_MAX) {
            static shared_ptr<Number> cache[CACHE_MAX - CACHE_MIN + 1];
            static bool init = false;
            if (!init) {
                for (long long i = CACHE_MIN; i <= CACHE_MAX; ++i)
                    cache[i - CACHE_MIN] = shared_ptr<Number>(new Number(i, false, false));
                init = true;
            }
            return cache[v - CACHE_MIN];
        }
        return shared_ptr<Number>(new Number(v, is_flt, is_bool));
    }


    DataType& set_pos(const optional<Position>& start, const optional<Position>& end) override {
        this->pos_start = start;
        this->pos_end = end;
        return *this;
    }

    DataType& set_context(const shared_ptr<Context>& ctx) override {
        this->context = ctx;
        return *this;
    }

    [[nodiscard]] bool is_number() const override { return true; }

    [[nodiscard]] bool is_truthy() const override {
        return std::visit([](auto val) { return val != 0; }, this->value);
    }

    [[nodiscard]] shared_ptr<DataType> copy() const override {
        return const_cast<Number*>(this)->shared_from_this();
    }

    [[nodiscard]] OperationResult is_true() const override {
        return std::make_pair(
            std::static_pointer_cast<DataType>(Number::make_bool(is_truthy())),
            nullptr
        );
    }

    [[nodiscard]] OperationResult add(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            bool res_is_float = this->is_float || other->is_float;
            variant<long long, double> new_value;
            if (res_is_float) {
                double left = std::visit([](auto val) -> double { return static_cast<double>(val); }, this->value);
                double right = std::visit([](auto val) -> double { return static_cast<double>(val); }, other->value);
                new_value = left + right;
            } else {
                new_value = std::visit([](auto left, auto right) -> variant<long long, double> {
                    if constexpr (is_integral_v<decltype(left)> && is_integral_v<decltype(right)>) {
                        return left + right;
                    }
                    return static_cast<double>(left) + static_cast<double>(right);
                    }, this->value, other->value);
            }

            shared_ptr<Number> result;
            if (holds_alternative<long long>(new_value) && !res_is_float)
                result = Number::make(get<long long>(new_value), false);
            else {
                double d_val = holds_alternative<double>(new_value) ? get<double>(new_value) : static_cast<double>(get<long long>(new_value));
                result = make_shared<Number>(d_val, res_is_float);
            }
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult subtract(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            bool res_is_float = this->is_float || other->is_float;
            variant<long long, double> new_value;
            if (res_is_float) {
                double left = std::visit([](auto val) -> double { return static_cast<double>(val); }, this->value);
                double right = std::visit([](auto val) -> double { return static_cast<double>(val); }, other->value);
                new_value = left - right;
            } else {
                new_value = std::visit([](auto left, auto right) -> variant<long long, double> {
                    if constexpr (is_integral_v<decltype(left)> && is_integral_v<decltype(right)>) {
                        return left - right;
                    }
                    return static_cast<double>(left) - static_cast<double>(right);
                    }, this->value, other->value);
            }

            shared_ptr<Number> result;
            if (holds_alternative<long long>(new_value) && !res_is_float)
                result = Number::make(get<long long>(new_value), false);
            else {
                double d_val = holds_alternative<double>(new_value) ? get<double>(new_value) : static_cast<double>(get<long long>(new_value));
                result = make_shared<Number>(d_val, res_is_float);
            }
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult multiply(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            bool res_is_float = this->is_float || other->is_float;
            variant<long long, double> new_value;
            if (res_is_float) {
                double left = std::visit([](auto val) -> double { return static_cast<double>(val); }, this->value);
                double right = std::visit([](auto val) -> double { return static_cast<double>(val); }, other->value);
                new_value = left * right;
            } else {
                new_value = std::visit([](auto left, auto right) -> variant<long long, double> {
                    if constexpr (is_integral_v<decltype(left)> && is_integral_v<decltype(right)>) {
                        return left * right;
                    }
                    return static_cast<double>(left) * static_cast<double>(right);
                    }, this->value, other->value);
            }

            shared_ptr<Number> result;
            if (holds_alternative<long long>(new_value) && !res_is_float)
                result = Number::make(get<long long>(new_value), false);
            else {
                double d_val = holds_alternative<double>(new_value) ? get<double>(new_value) : static_cast<double>(get<long long>(new_value));
                result = make_shared<Number>(d_val, res_is_float);
            }
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult divide(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            if (std::visit([](auto val) { return val == 0; }, other->value)) {
                return std::make_pair(nullptr, make_shared<DivisionByZeroError>(
                    other->pos_start.value_or(Position()),
                    other->pos_end.value_or(Position()),
                    "Division by zero",
                    this->context
                ));
            }
            double left = std::visit([](auto val) -> double { return static_cast<double>(val); }, this->value);
            double right = std::visit([](auto val) -> double { return static_cast<double>(val); }, other->value);
            auto result = make_shared<Number>(left / right, true);
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult modulus(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            if (std::visit([](auto val) { return val == 0; }, other->value)) {
                return std::make_pair(nullptr, make_shared<DivisionByZeroError>(
                    other->pos_start.value_or(Position()),
                    other->pos_end.value_or(Position()),
                    "Division by zero",
                    this->context
                ));
            }
            bool res_is_float = this->is_float || other->is_float;
            variant<long long, double> new_value;
            if (res_is_float) {
                double left = std::visit([](auto val) -> double { return static_cast<double>(val); }, this->value);
                double right = std::visit([](auto val) -> double { return static_cast<double>(val); }, other->value);
                new_value = fmod(left, right);
            } else {
                new_value = std::visit([](auto left, auto right) -> variant<long long, double> {
                    if constexpr (is_integral_v<decltype(left)> && is_integral_v<decltype(right)>) {
                        return left % right;
                    }
                    return fmod(static_cast<double>(left), static_cast<double>(right));
                    }, this->value, other->value);
            }

            shared_ptr<Number> result;
            if (holds_alternative<long long>(new_value) && !res_is_float)
                result = Number::make(get<long long>(new_value), false);
            else {
                double d_val = holds_alternative<double>(new_value) ? get<double>(new_value) : static_cast<double>(get<long long>(new_value));
                result = make_shared<Number>(d_val, res_is_float);
            }
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult floor_divide(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            if (std::visit([](auto val) { return val == 0; }, other->value)) {
                return std::make_pair(nullptr, make_shared<DivisionByZeroError>(
                    other->pos_start.value_or(Position()),
                    other->pos_end.value_or(Position()),
                    "Division by zero",
                    this->context
                ));
            }
            bool res_is_float = this->is_float || other->is_float;
            variant<long long, double> new_value;
            if (res_is_float) {
                double left = std::visit([](auto val) -> double { return static_cast<double>(val); }, this->value);
                double right = std::visit([](auto val) -> double { return static_cast<double>(val); }, other->value);
                new_value = floor(left / right);
            } else {
                new_value = std::visit([](auto left, auto right) -> variant<long long, double> {
                    if constexpr (is_integral_v<decltype(left)> && is_integral_v<decltype(right)>) {
                        return left / right;
                    }
                    return floor(static_cast<double>(left) / static_cast<double>(right));
                    }, this->value, other->value);
            }

            shared_ptr<Number> result;
            if (holds_alternative<long long>(new_value) && !res_is_float)
                result = Number::make(get<long long>(new_value), false);
            else {
                double d_val = holds_alternative<double>(new_value) ? get<double>(new_value) : static_cast<double>(get<long long>(new_value));
                result = make_shared<Number>(d_val, res_is_float);
            }
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult exponent(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            double left_val = std::visit([](auto val) -> double { return static_cast<double>(val); }, this->value);
            double right_val = std::visit([](auto val) -> double { return static_cast<double>(val); }, other->value);

            if (left_val == 0.0 && right_val < 0.0) {
                return std::make_pair(nullptr, make_shared<DivisionByZeroError>(
                    other->pos_start.value_or(Position()),
                    other->pos_end.value_or(Position()),
                    "Division by zero",
                    this->context
                ));
            }

            double res_d = pow(left_val, right_val);
            if (std::isnan(res_d) || std::isinf(res_d)) {
                return std::make_pair(nullptr, make_shared<IllegalOperationError>(
                    operand->pos_start.value_or(Position()),
                    operand->pos_end.value_or(Position()),
                    "Result too large or invalid exponentiation",
                    this->context
                ));
            }

            bool res_is_float = this->is_float || other->is_float || right_val < 0.0;
            shared_ptr<Number> result;
            if (!res_is_float && res_d >= static_cast<double>(LLONG_MIN) && res_d <= static_cast<double>(LLONG_MAX)) {
                result = Number::make(static_cast<long long>(std::round(res_d)), false);
            } else {
                result = make_shared<Number>(res_d, res_is_float);
            }
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult get_comparison_eq(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            bool result;
            if (this->is_float || other->is_float) {
                double left = std::visit([](auto val) -> double { return static_cast<double>(val); }, this->value);
                double right = std::visit([](auto val) -> double { return static_cast<double>(val); }, other->value);
                result = (round_to_13_dec(left) == round_to_13_dec(right));
            } else {
                result = std::visit([](auto left, auto right) { return left == right; }, this->value, other->value);
            }
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(result)), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult get_comparison_neq(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            bool result;
            if (this->is_float || other->is_float) {
                double left = std::visit([](auto val) -> double { return static_cast<double>(val); }, this->value);
                double right = std::visit([](auto val) -> double { return static_cast<double>(val); }, other->value);
                result = (round_to_13_dec(left) != round_to_13_dec(right));
            } else {
                result = std::visit([](auto left, auto right) { return left != right; }, this->value, other->value);
            }
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(result)), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult get_comparison_lt(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            bool result;
            if (this->is_float || other->is_float) {
                double left = std::visit([](auto val) -> double { return static_cast<double>(val); }, this->value);
                double right = std::visit([](auto val) -> double { return static_cast<double>(val); }, other->value);
                result = (round_to_13_dec(left) < round_to_13_dec(right));
            } else {
                result = std::visit([](auto left, auto right) { return left < right; }, this->value, other->value);
            }
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(result)), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult get_comparison_gt(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            bool result;
            if (this->is_float || other->is_float) {
                double left = std::visit([](auto val) -> double { return static_cast<double>(val); }, this->value);
                double right = std::visit([](auto val) -> double { return static_cast<double>(val); }, other->value);
                result = (round_to_13_dec(left) > round_to_13_dec(right));
            } else {
                result = std::visit([](auto left, auto right) { return left > right; }, this->value, other->value);
            }
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(result)), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult get_comparison_lte(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            bool result;
            if (this->is_float || other->is_float) {
                double left = std::visit([](auto val) -> double { return static_cast<double>(val); }, this->value);
                double right = std::visit([](auto val) -> double { return static_cast<double>(val); }, other->value);
                result = (round_to_13_dec(left) <= round_to_13_dec(right));
            } else {
                result = std::visit([](auto left, auto right) { return left <= right; }, this->value, other->value);
            }
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(result)), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult get_comparison_gte(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            bool result;
            if (this->is_float || other->is_float) {
                double left = std::visit([](auto val) -> double { return static_cast<double>(val); }, this->value);
                double right = std::visit([](auto val) -> double { return static_cast<double>(val); }, other->value);
                result = (round_to_13_dec(left) >= round_to_13_dec(right));
            } else {
                result = std::visit([](auto left, auto right) { return left >= right; }, this->value, other->value);
            }
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(result)), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult and_by(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            const bool self_truthy  = std::visit([](auto val) { return val != 0; }, this->value);
            const bool other_truthy = std::visit([](auto val) { return val != 0; }, other->value);
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(self_truthy && other_truthy)), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult or_by(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            const bool self_truthy  = std::visit([](auto val) { return val != 0; }, this->value);
            const bool other_truthy = std::visit([](auto val) { return val != 0; }, other->value);
            return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(self_truthy || other_truthy)), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult not_by() const override {
        const bool is_truthy = std::visit([](auto val) { return val != 0; }, this->value);
        return std::make_pair(std::static_pointer_cast<DataType>(Number::make_bool(!is_truthy)), nullptr);
    }

    [[nodiscard]] OperationResult bitwise_and(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            if (this->is_float || other->is_float) {
                return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Bitwise operations require integer Numbers", this->context));
            }
            long long left = holds_alternative<double>(this->value) ? static_cast<long long>(get<double>(this->value)) : get<long long>(this->value);
            long long right = holds_alternative<double>(other->value) ? static_cast<long long>(get<double>(other->value)) : get<long long>(other->value);
            auto result = Number::make(left & right, false);
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult bitwise_xor(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            if (this->is_float || other->is_float) {
                return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Bitwise operations require integer Numbers", this->context));
            }
            long long left = holds_alternative<double>(this->value) ? static_cast<long long>(get<double>(this->value)) : get<long long>(this->value);
            long long right = holds_alternative<double>(other->value) ? static_cast<long long>(get<double>(other->value)) : get<long long>(other->value);
            auto result = Number::make(left ^ right, false);
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult bitwise_or(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            if (this->is_float || other->is_float) {
                return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Bitwise operations require integer Numbers", this->context));
            }
            long long left = holds_alternative<double>(this->value) ? static_cast<long long>(get<double>(this->value)) : get<long long>(this->value);
            long long right = holds_alternative<double>(other->value) ? static_cast<long long>(get<double>(other->value)) : get<long long>(other->value);
            auto result = Number::make(left | right, false);
            result->set_context(this->context);
            return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult bitwise_not() const override {
        if (this->is_float) {
            return std::make_pair(nullptr, make_shared<IllegalOperationError>(this->pos_start.value_or(Position()), this->pos_end.value_or(Position()), "Bitwise operations require integer Numbers", this->context));
        }
        long long val = holds_alternative<double>(this->value) ? static_cast<long long>(get<double>(this->value)) : get<long long>(this->value);
        auto result = Number::make(~val, false);
        result->set_context(this->context);
        return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
    }

    [[nodiscard]] OperationResult lshift(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            if (this->is_float || other->is_float) {
                return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Bitwise operations require integer Numbers", this->context));
            }
            double left_d = std::visit([](auto val) -> double { return static_cast<double>(val); }, this->value);
            double right_d = std::visit([](auto val) -> double { return static_cast<double>(val); }, other->value);

            if (right_d < 0.0) {
                return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Negative shift count", this->context));
            }
            if (right_d >= 1000000.0) {
                return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Shift count too large or result overflow", this->context));
            }

            if (right_d >= 62.0 || holds_alternative<double>(this->value)) {
                double double_val = left_d * pow(2.0, right_d);
                auto result = make_shared<Number>(double_val, false);
                result->set_context(this->context);
                return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
            } else {
                long long left_i = get<long long>(this->value);
                long long right_i = static_cast<long long>(right_d);
                auto result = Number::make(left_i << right_i, false);
                result->set_context(this->context);
                return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
            }
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] OperationResult rshift(const shared_ptr<DataType>& operand) const override {
        if (operand->is_number()) {
            const auto other = static_cast<const Number*>(operand.get());
            if (this->is_float || other->is_float) {
                return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Bitwise operations require integer Numbers", this->context));
            }
            double left_d = std::visit([](auto val) -> double { return static_cast<double>(val); }, this->value);
            double right_d = std::visit([](auto val) -> double { return static_cast<double>(val); }, other->value);

            if (right_d < 0.0) {
                return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Negative shift count", this->context));
            }
            if (right_d >= 1000000.0) {
                return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Shift count too large or result overflow", this->context));
            }

            if (right_d >= 62.0 || holds_alternative<double>(this->value)) {
                double double_val = floor(left_d / pow(2.0, right_d));
                auto result = make_shared<Number>(double_val, false);
                result->set_context(this->context);
                return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
            } else {
                long long left_i = get<long long>(this->value);
                long long right_i = static_cast<long long>(right_d);
                auto result = Number::make(left_i >> right_i, false);
                result->set_context(this->context);
                return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
            }
        }
        return std::make_pair(nullptr, make_shared<IllegalOperationError>(operand->pos_start.value_or(Position()), operand->pos_end.value_or(Position()), "Expected a Number type", this->context));
    }

    [[nodiscard]] string to_string() const override {
        if (is_boolean) {
            return get<long long>(this->value) != 0 ? "True" : "False";
        }
        if (holds_alternative<double>(this->value)) {
            double d = get<double>(this->value);
            if (std::isinf(d)) {
                return d > 0 ? "INF" : "-INF";
            }
            if (std::isnan(d)) {
                return "NAN";
            }
            ostringstream ss;
            ss << std::fixed << std::setprecision(13) << d;
            string s = ss.str();
            if (s.find('.') != string::npos) {
                while (!s.empty() && s.back() == '0') {
                    s.pop_back();
                }
                if (!s.empty() && s.back() == '.') {
                    s += '0';
                }
            } else {
                s += ".0";
            }
            return s;
        }
        return std::to_string(get<long long>(this->value));
    }
};