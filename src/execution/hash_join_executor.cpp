//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hash_join_executor.cpp
//
// Identification: src/execution/hash_join_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/hash_join_executor.h"
#include "type/value_factory.h"

namespace bustub {

HashJoinExecutor::HashJoinExecutor(ExecutorContext *exec_ctx, const HashJoinPlanNode *plan,
                                   std::unique_ptr<AbstractExecutor> &&left_child,
                                   std::unique_ptr<AbstractExecutor> &&right_child)
    : AbstractExecutor(exec_ctx),plan_(plan),left_child_(std::move(left_child)),right_child_(std::move(right_child)) {
  if (!(plan->GetJoinType() == JoinType::LEFT || plan->GetJoinType() == JoinType::INNER)) {
    // Note for 2023 Fall: You ONLY need to implement left join and inner join.
    throw bustub::NotImplementedException(fmt::format("join type {} not supported", plan->GetJoinType()));
  }
  join_type_ = plan_->GetJoinType();
}

void HashJoinExecutor::Init() {
  this->left_child_->Init();
  this->right_child_->Init();
  jht_ = std::make_unique<SimpleHashJoinHashTable>();
  Tuple tuple_buffer{};
  RID rid_buffer{};
  while (this->right_child_->Next(&tuple_buffer, &rid_buffer)){
    jht_->InsertKey(MakeHashRightJoinKey(&tuple_buffer), tuple_buffer);
  }
  left_child_->Next(&current_left_tuple_, &current_left_rid_);
  current_tuple_vector_ = jht_->GetValue(MakeHashLeftJoinKey(&current_left_tuple_));
  if (current_tuple_vector_ != nullptr) {
    current_vector_it_ = current_tuple_vector_->begin();
    current_left_tuple_matched_ = true;
  }
}

auto HashJoinExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  Tuple tuple_buffer{};
  while (true){
    if (current_tuple_vector_ != nullptr && current_vector_it_ != current_tuple_vector_->end()){
      auto current_right_tuple = *current_vector_it_;
      std::vector<Value> values;
      for (uint32_t i = 0; i < left_child_->GetOutputSchema().GetColumnCount(); ++i) {
        values.emplace_back(this->current_left_tuple_.GetValue(&left_child_->GetOutputSchema(), i));
      }
      for (uint32_t i = 0; i < right_child_->GetOutputSchema().GetColumnCount(); ++i) {
        values.emplace_back(current_right_tuple.GetValue(&right_child_->GetOutputSchema(), i));
      }
      *tuple = Tuple{values, &GetOutputSchema()};
      current_vector_it_++;
      return true;
    }
    if (join_type_ == JoinType::LEFT){
      if (!current_left_tuple_matched_){
        std::vector<Value> values;
        for (uint32_t i = 0; i < left_child_->GetOutputSchema().GetColumnCount(); ++i) {
          values.emplace_back(this->current_left_tuple_.GetValue(&left_child_->GetOutputSchema(), i));
        }
        for (uint32_t i = 0; i < right_child_->GetOutputSchema().GetColumnCount(); ++i) {
          values.emplace_back(ValueFactory::GetNullValueByType(right_child_->GetOutputSchema().GetColumn(i).GetType()));
        }
        *tuple = Tuple{values, &GetOutputSchema()};
        current_left_tuple_matched_ = true;
        return true;
      }
    }
    if (!left_child_->Next(&current_left_tuple_, &current_left_rid_)) {
      return false;
    }
    current_left_tuple_matched_ = false;
    current_tuple_vector_ = jht_->GetValue(MakeHashLeftJoinKey(&current_left_tuple_));
    if (current_tuple_vector_ != nullptr) {
      current_vector_it_ = current_tuple_vector_->begin();
      current_left_tuple_matched_ = true;
    }
  }
}

}  // namespace bustub
