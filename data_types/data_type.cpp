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
