#include "execution/executors/window_function_executor.h"
#include "execution/plans/window_plan.h"
#include "storage/table/tuple.h"
#include "execution/executors/sort_executor.h"
#include "execution/expressions/column_value_expression.h"
#include "execution/plans/aggregation_plan.h"
#include "type/value_factory.h"

namespace bustub {
auto GenerateInitialAggregateValue(WindowFunctionType wft) -> Value {
  Value result;
  switch (wft) {
    case WindowFunctionType::CountStarAggregate:
      result = ValueFactory::GetIntegerValue(0);
      break;
    case WindowFunctionType::CountAggregate:
    case WindowFunctionType::SumAggregate:
    case WindowFunctionType::MinAggregate:
    case WindowFunctionType::MaxAggregate:
    case WindowFunctionType::Rank:
      result = ValueFactory::GetNullValueByType(TypeId::INTEGER);
      break;
  }
  return result;
}
auto WindowFunctionExecutor::MakeAggregateKey(const Tuple *tuple,
                                              const std::vector<AbstractExpressionRef> &partition_by) -> AggregateKey {
  std::vector<Value> keys;
  keys.reserve(partition_by.size());
  for (const auto &expr : partition_by) {
    keys.emplace_back(expr->Evaluate(tuple, child_executor_->GetOutputSchema()));
  }
  return {keys};
}

WindowFunctionExecutor::WindowFunctionExecutor(ExecutorContext *exec_ctx, const WindowFunctionPlanNode *plan,
                                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void WindowFunctionExecutor::Init() {
  child_executor_->Init();
  Tuple tuple_buffer{};
  RID rid_buffer{};
  while (child_executor_->Next(&tuple_buffer, &rid_buffer)) {
    tuples_buffer_.emplace_back(tuple_buffer);
    output_.emplace_back(std::vector<Value>());
  }
  for (auto &tuple : output_) {
    tuple.reserve(plan_->columns_.size());
    for (uint32_t i = 0; i < plan_->columns_.size(); ++i) {
      tuple.emplace_back(Value{});
    }
  }
  if (!plan_->window_functions_.empty()){
    const auto order_bys = (plan_->window_functions_.begin())->second.order_by_;
    std::sort(tuples_buffer_.begin(), tuples_buffer_.end(), TupleComparator(&this->GetOutputSchema(), order_bys));
  }
  if (!plan_->window_functions_.empty()){
    if (!plan_->window_functions_.begin()->second.order_by_.empty()){
       for (auto const &pair : plan_->window_functions_){
        auto wf_idx = pair.first;
        auto wf = pair.second;
        AbstractExpressionRef func = wf.function_;
        WindowFunctionType wft = wf.type_;
        std::vector<AbstractExpressionRef> partition_by = wf.partition_by_;
        std::unordered_map<AggregateKey, Value> partition_value_buffer;
        for (uint32_t i = 0; i < tuples_buffer_.size(); i++){
          auto tuple = tuples_buffer_[i];
          auto agg_val = MakeAggregateKey(&tuple, partition_by);
          auto par_val = func->Evaluate(&tuple, child_executor_->GetOutputSchema());
          if (partition_value_buffer.count(agg_val) == 0) {
            partition_value_buffer.insert({agg_val, GenerateInitialAggregateValue(wft)});
          }
          Value &temp_value = partition_value_buffer[agg_val];
          switch (wft) {
            case WindowFunctionType::CountStarAggregate:
              temp_value = temp_value.Add(Value{INTEGER, 1});

              break;
            case WindowFunctionType::Rank:
            case WindowFunctionType::CountAggregate:
              if (!par_val.IsNull()) {
                if (temp_value.IsNull()) {
                  temp_value = ValueFactory::GetIntegerValue(0);
                }
                temp_value = temp_value.Add(Value{INTEGER, 1});
              }
              break;
            case WindowFunctionType::SumAggregate:
              if (!par_val.IsNull()) {
                if (temp_value.IsNull()) {
                  temp_value = par_val;
                } else {
                  temp_value = temp_value.Add(par_val);
                }
              }
              break;
            case WindowFunctionType::MinAggregate:
              if (!par_val.IsNull()) {
                if (temp_value.IsNull()) {
                  temp_value = par_val;
                } else {
                  temp_value = par_val.CompareLessThan(temp_value) == CmpBool::CmpTrue ? par_val.Copy() : temp_value;
                }
              }
              break;
            case WindowFunctionType::MaxAggregate:
              if (!par_val.IsNull()) {
                if (temp_value.IsNull()) {
                  temp_value = par_val;
                } else {
                  temp_value = par_val.CompareGreaterThan(temp_value) == CmpBool::CmpTrue ? par_val.Copy() : temp_value;
                }
              }
              break;
          }
          if (i > 0) {
            if (IsTupleContentEqual(tuple, tuples_buffer_[i - 1])) {
              output_[i] = output_[i - 1];
              continue;
            }
          }
          output_[i][wf_idx] = temp_value;
        }
       } 
    }else{
        for (auto const &pair : plan_->window_functions_){
          auto wf_idx = pair.first;
          auto wf = pair.second;
          AbstractExpressionRef func = wf.function_;
          WindowFunctionType wft = wf.type_;
          std::vector<AbstractExpressionRef> partition_by = wf.partition_by_;
          std::unordered_map<AggregateKey, Value> partition_value_buffer;
          for (auto const &tuple : tuples_buffer_){
            auto agg_val = MakeAggregateKey(&tuple, partition_by);
            auto par_val = func->Evaluate(&tuple, child_executor_->GetOutputSchema());
            if (partition_value_buffer.count(agg_val) == 0) {
              partition_value_buffer.insert({agg_val, GenerateInitialAggregateValue(wft)});
            }
            Value &temp_value = partition_value_buffer[agg_val];
            switch (wft){
              case WindowFunctionType::CountStarAggregate:
                temp_value = temp_value.Add(Value{INTEGER, 1});
                break;  
              case WindowFunctionType::CountAggregate:
                if (!par_val.IsNull()) {
                    if (temp_value.IsNull()) {
                    temp_value = ValueFactory::GetIntegerValue(0);
                    }
                    temp_value = temp_value.Add(Value{INTEGER, 1});
                }
                break;
              case WindowFunctionType::SumAggregate:
                if (!par_val.IsNull()) {
                    if (temp_value.IsNull()) {
                    temp_value = par_val;
                    } else {
                    temp_value = temp_value.Add(par_val);
                    }
                }
                break;
              case WindowFunctionType::MinAggregate:
                if (!par_val.IsNull()) {
                  if (temp_value.IsNull()) {
                    temp_value = par_val;
                  } else {
                    temp_value = par_val.CompareLessThan(temp_value) == CmpBool::CmpTrue ? par_val.Copy() : temp_value;
                  }
                }
                break;
              case WindowFunctionType::MaxAggregate:
                if (!par_val.IsNull()) {
                  if (temp_value.IsNull()) {
                    temp_value = par_val;
                  } else {
                    temp_value = par_val.CompareGreaterThan(temp_value) == CmpBool::CmpTrue ? par_val.Copy() : temp_value;
                  }
                }
                break;
              case WindowFunctionType::Rank:
                break;
            }
          }
          for (uint32_t i = 0; i < output_.size(); ++i){
            auto agg_val = MakeAggregateKey(&tuples_buffer_[i], partition_by);
            if (wft == WindowFunctionType::Rank){
                if (partition_value_buffer.count(agg_val) == 0) {
                    partition_value_buffer.insert({agg_val, ValueFactory::GetIntegerValue(0)});
                }
                partition_value_buffer.at(agg_val) = partition_value_buffer.at(agg_val).Add(Value{INTEGER, 1});
                output_[i][wf_idx] = (partition_value_buffer.at(agg_val));
            }else{
                output_[i][wf_idx] = (partition_value_buffer.at(agg_val));
            }
          }
        }
    }
  }
  tuple_buffer_it_ = tuples_buffer_.begin();
  output_it_ = output_.begin();
}

auto WindowFunctionExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  while (tuple_buffer_it_ != tuples_buffer_.end()){
    std::vector<Value> values;
    for (auto const &col : plan_->columns_) {
      auto *col_value = dynamic_cast<ColumnValueExpression *>(col.get());
      uint32_t col_idx = col_value->GetColIdx();
      if (col_idx < this->GetOutputSchema().GetColumnCount()) {
        Value val = col_value->Evaluate(&(*tuple_buffer_it_), child_executor_->GetOutputSchema());
        (*output_it_)[col_idx] = val;
      }
    }
    *tuple = Tuple{(*output_it_), &this->GetOutputSchema()};
    tuple_buffer_it_++;
    output_it_++;
    return true;
  }
  return false;
}
}  // namespace bustub
