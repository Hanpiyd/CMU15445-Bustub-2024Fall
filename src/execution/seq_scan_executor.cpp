//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// seq_scan_executor.cpp
//
// Identification: src/execution/seq_scan_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/seq_scan_executor.h"

namespace bustub {

SeqScanExecutor::SeqScanExecutor(ExecutorContext *exec_ctx, const SeqScanPlanNode *plan)
 : AbstractExecutor(exec_ctx),plan_(plan){}

void SeqScanExecutor::Init() {
    auto table_info = exec_ctx_->GetCatalog()->GetTable(plan_->table_name_);
    table_iter_ = std::make_unique<TableIterator>(table_info->table_->MakeIterator());
}

auto SeqScanExecutor::Next(Tuple *tuple, RID *rid) -> bool {
    bool flag = false;
    auto filter_expr = plan_->filter_predicate_;
    while(!table_iter_->IsEnd()){
        std::pair<TupleMeta,Tuple>tuple_data = table_iter_->GetTuple();
        if(!tuple_data.first.is_deleted_){
            if(filter_expr != nullptr){
                auto value = filter_expr->Evaluate(&tuple_data.second, this->GetOutputSchema());
                if (!value.IsNull() && value.GetAs<bool>()){
                     *tuple = Tuple{std::move(tuple_data.second)};
                     if (rid != nullptr){
                        *rid = RID{table_iter_->GetRID().Get()};
                     }
                     flag = true;
                     ++(*table_iter_);
                     break;
                }
            }
            else{
                *tuple = Tuple{std::move(tuple_data.second)};
                if (rid != nullptr){
                    *rid = RID{table_iter_->GetRID().Get()};
                }
                flag = true;
                ++(*table_iter_);
                break;
            }
        }
        ++(*table_iter_);
    }
    return flag;
}

}  // namespace bustub
