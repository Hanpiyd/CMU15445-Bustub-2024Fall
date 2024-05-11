#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <queue>
#include <valarray>
#include <vector>
#include "common/logger.h"
#include "concurrency/transaction.h"
#include "execution/expressions/abstract_expression.h"
#include "execution/expressions/column_value_expression.h"
#include "execution/expressions/comparison_expression.h"
#include "execution/expressions/constant_value_expression.h"
#include "execution/expressions/logic_expression.h"
#include "execution/plans/abstract_plan.h"
#include "execution/plans/index_scan_plan.h"
#include "execution/plans/seq_scan_plan.h"
#include "optimizer/optimizer.h"
#include "type/vector_type.h"

namespace bustub {

auto Optimizer::OptimizeSeqScanAsIndexScan(const bustub::AbstractPlanNodeRef &plan) -> AbstractPlanNodeRef {
  std::vector<AbstractPlanNodeRef> children;
  std::vector<AbstractPlanNodeRef> logic_children;
  std::vector<index_oid_t> index_ids;
  for (const auto &child : plan->GetChildren()) {
    children.emplace_back(OptimizeSeqScanAsIndexScan(child));
  }
  auto optimized_plan = plan->CloneWithChildren(std::move(children));
  if (optimized_plan->GetType() == PlanType::SeqScan) {
    const auto &seq_scan = dynamic_cast<const SeqScanPlanNode &>(*optimized_plan);
    if (seq_scan.filter_predicate_ != nullptr) {
      const auto *logic_expr = dynamic_cast<const LogicExpression *>(seq_scan.filter_predicate_.get());
      if (logic_expr != nullptr && logic_expr->logic_type_ == LogicType::Or) {
        const auto *first_cmp_expr = dynamic_cast<const ComparisonExpression *>(logic_expr->children_[0].get());
        if (first_cmp_expr != nullptr) {
          auto *first_column_value_expr = dynamic_cast<ColumnValueExpression *>(first_cmp_expr->children_[0].get());
          if (first_column_value_expr == nullptr) {
            first_column_value_expr = dynamic_cast<ColumnValueExpression *>(first_cmp_expr->children_[1].get());
          }
          if (first_column_value_expr != nullptr) {
            uint32_t first_col_idx = first_column_value_expr->GetColIdx();
            auto children = logic_expr->children_;
            children.clear();
            std::queue<std::shared_ptr<AbstractExpression>> q;
            for (const auto &i : logic_expr->children_) {
              q.push(i);
            }
            while (!q.empty()) {
              auto node = q.front();
              q.pop();
              auto expr = dynamic_cast<const ComparisonExpression *>(node.get());
              if (expr != nullptr) {
                children.push_back(node);
              } else {
                auto left = node->children_[0];
                auto right = node->children_[1];
                q.push(left);
                q.push(right);
              }
            }

            for (const auto &i : children) {
              const auto *cmp_expr = dynamic_cast<const ComparisonExpression *>(i.get());
              if (cmp_expr != nullptr) {
                auto *column_value_expr = dynamic_cast<ColumnValueExpression *>(cmp_expr->children_[0].get());
                if (column_value_expr == nullptr) {
                  column_value_expr = dynamic_cast<ColumnValueExpression *>(cmp_expr->children_[1].get());
                }
                if (column_value_expr != nullptr) {
                  uint32_t col_idx = column_value_expr->GetColIdx();
                  if (col_idx != first_col_idx) {
                    return optimized_plan;
                  }
                }
              }
            }
          }
        }
        auto children = logic_expr->children_;
        children.clear();
        std::queue<std::shared_ptr<AbstractExpression>> q;
        for (const auto &i : logic_expr->children_) {
          q.push(i);
        }
        while (!q.empty()) {
          auto node = q.front();
          q.pop();
          auto expr = dynamic_cast<const ComparisonExpression *>(node.get());
          if (expr != nullptr) {
            children.push_back(node);
          } else {
            auto left = node->children_[0];
            auto right = node->children_[1];
            q.push(left);
            q.push(right);
          }
        }
        for (const auto &child_expr : children) {
          std::cout << child_expr->ToString() << std::endl;
          const auto *cmp_expr = dynamic_cast<const ComparisonExpression *>(child_expr.get());
          if (cmp_expr != nullptr && cmp_expr->comp_type_ == ComparisonType::Equal) {
            const auto *table_info = catalog_.GetTable(seq_scan.GetTableOid());
            const auto indices = catalog_.GetTableIndexes(table_info->name_);
            auto *column_value_expr = dynamic_cast<ColumnValueExpression *>(cmp_expr->children_[0].get());
            if (column_value_expr == nullptr) {
              column_value_expr = dynamic_cast<ColumnValueExpression *>(cmp_expr->children_[1].get());
            }
            if (column_value_expr != nullptr) {
              int i = 0;
              for (const auto *index : indices) {
                std::cout << i++ << std::endl;
                const auto &columns = index->index_->GetKeyAttrs();
                std::vector<uint32_t> filter_column_ids = {column_value_expr->GetColIdx()};
                if (filter_column_ids == columns) {
                  if (index_ids.empty()) {
                    index_ids.push_back(index->index_oid_);
                  } else {
                    if (index->index_oid_ != index_ids[0]) {
                      return optimized_plan;
                    }
                  }
                  logic_children.emplace_back(std::make_shared<IndexScanPlanNode>(
                      optimized_plan->output_schema_, table_info->oid_, index->index_oid_, seq_scan.filter_predicate_));

                  break;
                }
              }
            }
          }
        }
        std::cout << "==================\n";
        return plan->CloneWithChildren(std::move(logic_children));
      }
      if (logic_expr == nullptr) {
        const auto *cmp_expr = dynamic_cast<const ComparisonExpression *>(seq_scan.filter_predicate_.get());
        if (cmp_expr != nullptr && cmp_expr->comp_type_ == ComparisonType::Equal) {
          const auto *table_info = catalog_.GetTable(seq_scan.GetTableOid());
          const auto indices = catalog_.GetTableIndexes(table_info->name_);
          auto *column_value_expr = dynamic_cast<ColumnValueExpression *>(cmp_expr->children_[0].get());
          if (column_value_expr == nullptr) {
            column_value_expr = dynamic_cast<ColumnValueExpression *>(cmp_expr->children_[1].get());
          }
          if (column_value_expr != nullptr) {
            for (const auto *index : indices) {
              const auto &columns = index->index_->GetKeyAttrs();
              std::vector<uint32_t> filter_column_ids = {column_value_expr->GetColIdx()};
              if (filter_column_ids == columns) {
                return std::make_shared<IndexScanPlanNode>(optimized_plan->output_schema_, table_info->oid_,
                                                           index->index_oid_, seq_scan.filter_predicate_);
              }
            }
          }
        }
      }
    }
  }
  return optimized_plan;
}  


}