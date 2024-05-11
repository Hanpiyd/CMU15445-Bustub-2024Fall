//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// delete_executor.cpp
//
// Identification: src/execution/delete_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <memory>

#include "execution/executors/delete_executor.h"

namespace bustub {

DeleteExecutor::DeleteExecutor(ExecutorContext *exec_ctx, const DeletePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx),plan_(plan),child_executor_(std::move(child_executor)) {}

void DeleteExecutor::Init() {
    this->child_executor_->Init();
    Catalog *catalog = exec_ctx_->GetCatalog();
    table_info_ = catalog->GetTable(plan_->table_oid_);
    index_infos_ = catalog->GetTableIndexes(table_info_->name_);
    table_heap_ = table_info_->table_.get();
}

auto DeleteExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
    Tuple child_tuple{};
    int count = 0;
    RID child_rid{};
    while(child_executor_->Next(&child_tuple, &child_rid)){
        TupleMeta old_tuple_meta = table_heap_->GetTupleMeta(child_rid);
        old_tuple_meta.is_deleted_ = true;
        table_heap_->UpdateTupleMeta(old_tuple_meta, child_rid);
        auto it = index_infos_.begin();
        for(; it != index_infos_.end(); ++it){
            auto key_schema = (*it)->index_->GetKeySchema();
            auto key_attr = (*it)->index_->GetKeyAttrs();
            (*it)->index_->DeleteEntry(child_tuple.KeyFromTuple(table_info_->schema_, *key_schema, key_attr), child_rid,
                                       exec_ctx_->GetTransaction());
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
