#include "optimizer/optimizer.h"
#include <execution/plans/limit_plan.h>
#include <execution/plans/sort_plan.h>
#include <execution/plans/topn_plan.h>

namespace bustub {

auto Optimizer::OptimizeSortLimitAsTopN(const AbstractPlanNodeRef &plan) -> AbstractPlanNodeRef {
  // TODO(student): implement sort + limit -> top N optimizer rule
  std::vector<AbstractPlanNodeRef> children;
  for (const auto &child : plan->GetChildren()) {
    children.emplace_back(OptimizeSortLimitAsTopN(child));
  }
  auto optimized_plan = plan->CloneWithChildren(std::move(children));
  if (optimized_plan->GetType() == PlanType::Limit){
    const auto &limit_plan = dynamic_cast<const LimitPlanNode &>(*optimized_plan);
    auto abstract_plan_node_ref = limit_plan.GetChildPlan();
    if (abstract_plan_node_ref->GetType() == PlanType::Sort){
      const auto &sort_plan = dynamic_cast<const SortPlanNode &>(*abstract_plan_node_ref.get());
      return std::make_shared<TopNPlanNode>(sort_plan.output_schema_, sort_plan.GetChildPlan(), sort_plan.GetOrderBy(),
                                            limit_plan.GetLimit());
    }
  }
  return optimized_plan;
}

}  // namespace bustub
