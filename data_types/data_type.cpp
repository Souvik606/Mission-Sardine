#include "data_type.h"
#include "number_type.h"

DataType::OperationResult BoundMethod::is_true() const {
    auto result = make_shared<Number>(1LL);
    result->set_context(this->context);
    result->set_pos(this->pos_start, this->pos_end);
    return std::make_pair(std::static_pointer_cast<DataType>(result), nullptr);
}

shared_ptr<DataType> BoundMethod::copy() const {
    auto new_bound = make_shared<BoundMethod>(this->name, this->instance, this->execute_impl);
    new_bound->set_pos(this->pos_start, this->pos_end);
    new_bound->set_context(this->context);
    return new_bound;
}

DataType::OperationResult BoundMethod::get_comparison_eq(const shared_ptr<DataType>& other) const {
    if (other->get_type_name() == "Null") {
        return { Number::make_bool(false), nullptr };
    }
    bool eq = (this == other.get());
    return { Number::make_bool(eq), nullptr };
}

DataType::OperationResult BoundMethod::get_comparison_neq(const shared_ptr<DataType>& other) const {
    if (other->get_type_name() == "Null") {
        return { Number::make_bool(true), nullptr };
    }
    bool neq = (this != other.get());
    return { Number::make_bool(neq), nullptr };
}

#include "model_type.h"
#include "string_type.h"

UserDefinedError::UserDefinedError(const Position& pos_start, const Position& pos_end, shared_ptr<DataType> inst, shared_ptr<Context> context, string hint)
    : RunTimeError(pos_start, pos_end, get_details_from_instance(inst), std::move(context), get_name_from_instance(inst), "E9007", std::move(hint)), instance(std::move(inst)) {
}

string UserDefinedError::get_name_from_instance(const shared_ptr<DataType>& inst) {
    if (auto model_inst = dynamic_pointer_cast<ModelInstance>(inst)) {
        return model_inst->model->name;
    }
    return "UserDefinedError";
}

string UserDefinedError::get_details_from_instance(const shared_ptr<DataType>& inst) {
    if (auto model_inst = dynamic_pointer_cast<ModelInstance>(inst)) {
        if (model_inst->symbol_table) {
            auto msg_val = model_inst->symbol_table->get("message");
            if (msg_val) {
                if (auto str_val = dynamic_pointer_cast<String>(msg_val)) {
                    return str_val->value;
                }
                return msg_val->to_string();
            }
        }
        return "<" + model_inst->model->name + " instance>";
    }
    return "";
}
