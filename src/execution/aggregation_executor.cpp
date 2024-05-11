//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// aggregation_executor.cpp
//
// Identification: src/execution/aggregation_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include <memory>
#include <vector>

#include "execution/executors/aggregation_executor.h"

namespace bustub {

AggregationExecutor::AggregationExecutor(ExecutorContext *exec_ctx, const AggregationPlanNode *plan,
                                         std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx),plan_(plan),child_executor_(std::move(child_executor)) {}

void AggregationExecutor::Init() {
    child_executor_->Init();
    aht_ = std::make_unique<SimpleAggregationHashTable>(plan_->GetAggregates(), plan_->GetAggregateTypes());
    Tuple tuple_buffer{};
    RID rid{};
    while (child_executor_->Next(&tuple_buffer, &rid)) {
        AggregateKey key = this->MakeAggregateKey(&tuple_buffer);
        AggregateValue value = this->MakeAggregateValue(&tuple_buffer);
        aht_->InsertCombine(key, value);
    }
    aht_iterator_ = std::make_unique<SimpleAggregationHashTable::Iterator>(aht_->Begin());
}

auto AggregationExecutor::Next(Tuple *tuple, RID *rid) -> bool {
    if (aht_->Begin() == aht_->End() && !is_end_){
        if (!plan_->GetGroupBys().empty()) {
            return false;
        }
        std::vector<Value> values{};
        Tuple tuple_buffer{};
        AggregateKey agg_key = this->MakeAggregateKey(&tuple_buffer);
        AggregateValue agg_val = aht_->GenerateInitialAggregateValue();
        for (auto &group_by : agg_key.group_bys_){
            values.emplace_back(group_by);
        }
        for (auto &aggregate : agg_val.aggregates_){
            values.emplace_back(aggregate);
        }
        *tuple = {values, &GetOutputSchema()};
        is_end_ = true;
        return true;
    }
    if(*aht_iterator_ != aht_->End()){
        std::vector<Value> values{};
        AggregateKey agg_key = aht_iterator_->Key();
        AggregateValue agg_val = aht_iterator_->Val();
        for (auto &group_by : agg_key.group_bys_) {
            values.emplace_back(group_by);
        }
        for (auto &aggregate : agg_val.aggregates_) {
            values.emplace_back(aggregate);
        }
        *tuple = {values, &GetOutputSchema()};
        ++(*aht_iterator_);
    }else{
        is_end_ = true;
        return false;
    }
    return true;
}

auto AggregationExecutor::GetChildExecutor() const -> const AbstractExecutor * { return child_executor_.get(); }

}  // namespace bustub
