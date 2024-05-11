#include "execution/executors/topn_executor.h"

namespace bustub {

TopNExecutor::TopNExecutor(ExecutorContext *exec_ctx, const TopNPlanNode *plan,
                           std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void TopNExecutor::Init() {
  BUSTUB_ENSURE(plan_->GetN() > 0, "The N of Limit is must larger than 0!");
  child_executor_->Init();
  tuple_container_ = std::priority_queue<Tuple, std::vector<Tuple>, CompareRule>(
      CompareRule(&this->GetOutputSchema(), plan_->GetOrderBy()));
  Tuple tuple_buffer{};
  RID rid_buffer{};
  while (child_executor_->Next(&tuple_buffer, &rid_buffer)) {
    tuple_container_.push(tuple_buffer);
    if (GetNumInHeap() > plan_->GetN()) {
      tuple_container_.pop();
    }
  }
  while (!tuple_container_.empty()) {
    stack_.push(tuple_container_.top());
    tuple_container_.pop();
  }   
}

auto TopNExecutor::Next(Tuple *tuple, RID *rid) -> bool {
   while (!stack_.empty()) {
    *tuple = stack_.top();
    stack_.pop();
    return true;
  }
  return false;
}

auto TopNExecutor::GetNumInHeap() -> size_t {return tuple_container_.size();};

}  // namespace bustub
