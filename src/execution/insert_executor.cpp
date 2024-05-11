//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// insert_executor.cpp
//
// Identification: src/execution/insert_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <memory>

#include "execution/executors/insert_executor.h"

namespace bustub {

InsertExecutor::InsertExecutor(ExecutorContext *exec_ctx, const InsertPlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx),plan_(plan),child_executor_(std::move(child_executor)) {}

void InsertExecutor::Init() {
    this->child_executor_->Init();
    Catalog *catalog = exec_ctx_->GetCatalog();
    table_info_ = catalog->GetTable(plan_->table_oid_);
    index_infos_ = catalog->GetTableIndexes(table_info_->name_);
    table_heap_ = table_info_->table_.get();
}

auto InsertExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
    Tuple child_tuple{};
    RID child_rid{};
    int count = 0;
    while(child_executor_->Next(&child_tuple, &child_rid)){
        std::optional<RID> new_rid_optional = table_heap_->InsertTuple(TupleMeta{0, false}, child_tuple);
        BUSTUB_ENSURE(new_rid_optional.has_value(), "can not insert new tuple, no rid!");
        RID new_rid = new_rid_optional.value();
        auto it = index_infos_.begin();
        for (; it != index_infos_.end(); ++it) {
        auto key_schema = (*it)->index_->GetKeySchema();
        auto key_attr = (*it)->index_->GetKeyAttrs();
        bool is_success = (*it)->index_->InsertEntry(child_tuple.KeyFromTuple(table_info_->schema_, *key_schema, key_attr), new_rid, exec_ctx_->GetTransaction());
        if (!is_success) {
            std::vector<Value> value{};
            value.emplace_back(INTEGER, count);
            *tuple = Tuple{value, &GetOutputSchema()};
            is_end_ = true;
            return true;
        }
    }
    count++;
    }
    std::vector<Value> value{}; 
    value.emplace_back(INTEGER, count);
    *tuple = Tuple{value, &GetOutputSchema()};
    if(is_end_){
        return false;
    }
    is_end_ = true;
    return true;
}

}  // namespace bustub
