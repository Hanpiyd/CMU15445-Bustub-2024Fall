#include "execution/executors/sort_executor.h"
#include "execution/expressions/column_value_expression.h"

namespace bustub {

SortExecutor::SortExecutor(ExecutorContext *exec_ctx, const SortPlanNode *plan,
                           std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void SortExecutor::Init() {
  child_executor_->Init();
  Tuple tuple_buffer{};
  RID rid_buffer{};
  while (child_executor_->Next(&tuple_buffer, &rid_buffer)) {
    tuples_.emplace_back(tuple_buffer);
  }
  const auto order_bys = plan_->GetOrderBy();
  std::sort(tuples_.begin(), tuples_.end(), TupleComparator(&this->GetOutputSchema(), order_bys));
  current_tuple_it_ = tuples_.begin();
}

auto SortExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  while (current_tuple_it_ != tuples_.end()) {
    *tuple = *current_tuple_it_;
    ++current_tuple_it_;
    return true;
  }
  return false;
}

}  // namespace bustub
