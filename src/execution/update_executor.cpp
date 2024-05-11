//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// update_executor.cpp
//
// Identification: src/execution/update_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include <memory>

#include "execution/executors/update_executor.h"

namespace bustub {

UpdateExecutor::UpdateExecutor(ExecutorContext *exec_ctx, const UpdatePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx),plan_(plan),child_executor_(std::move(child_executor)) {
  // As of Fall 2022, you DON'T need to implement update executor to have perfect score in project 3 / project 4.
  // But now it is 2024 Spring, I have to FINISH this.
}

void UpdateExecutor::Init() {
  this->child_executor_->Init();
  Catalog *catalog = exec_ctx_->GetCatalog();
  table_info_ = catalog->GetTable(plan_->table_oid_);
  index_infos_ = catalog->GetTableIndexes(table_info_->name_);
}

auto UpdateExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
  Tuple child_tuple{};
  int count = 0;
  RID child_rid{};
  TableHeap *table_heap = table_info_->table_.get();
  std::vector<AbstractExpressionRef> target_exprs = plan_->target_expressions_;
  while(child_executor_->Next(&child_tuple, &child_rid)){
    TupleMeta old_tuple_meta = table_heap->GetTupleMeta(child_rid);
    old_tuple_meta.is_deleted_ = true;
    table_heap->UpdateTupleMeta(old_tuple_meta, child_rid);
    std::vector<Value> new_value{};
    for(auto &target_expr : target_exprs){
      Value value = target_expr->Evaluate(&child_tuple, child_executor_->GetOutputSchema());
      new_value.emplace_back(value);
    }
    Tuple new_tuple = Tuple{new_value, &(child_executor_->GetOutputSchema())};
    std::optional<RID> new_rid_optional = table_heap->InsertTuple(TupleMeta{0, false}, new_tuple);
    BUSTUB_ENSURE(new_rid_optional.has_value(), "can not update tuple, no rid!");
    RID new_rid = new_rid_optional.value();
    auto it = index_infos_.begin();
    for(; it != index_infos_.end(); ++it){
      auto key_schema = (*it)->index_->GetKeySchema();
      auto key_attr = (*it)->index_->GetKeyAttrs();
      (*it)->index_->DeleteEntry(child_tuple.KeyFromTuple(table_info_->schema_, *key_schema, key_attr), child_rid,
                                 exec_ctx_->GetTransaction());
      bool is_success = (*it)->index_->InsertEntry(new_tuple.KeyFromTuple(table_info_->schema_, *key_schema, key_attr),
                                                   new_rid, exec_ctx_->GetTransaction());
      if(!is_success){
        std::vector<Value> value{};
        value.emplace_back(INTEGER, count);
        *tuple = Tuple{value, &GetOutputSchema()};
        return false;
      }                                             
    }
    count++;
  }
  std::vector<Value> value{};
  value.emplace_back(INTEGER, count);
  *tuple = Tuple{value, &GetOutputSchema()};
  if (is_end_) {
    return false;
  }
  is_end_ = true;
  return true;
}

}  // namespace bustub
