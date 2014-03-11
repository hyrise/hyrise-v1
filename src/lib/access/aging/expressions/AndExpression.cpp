// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "AndExpression.h"

#include <unordered_set>

#include <access/expressions/pred_CompoundExpression.h>
#include <access/expressions/pred_TrueExpression.h>

namespace hyrise {
namespace access {
namespace aging {

AndExpression::AndExpression(const std::vector<std::shared_ptr<SelectExpression>>& subExpressions) :
    _subExpressions(subExpressions) {}

SimpleExpression* AndExpression::expression(storage::atable_ptr_t table,
                                            const std::map<storage::field_t, std::vector<storage::value_id_t>>& vids) const {
  std::vector<SimpleExpression*> subExprs;
  for (const auto& subExpression : _subExpressions) {
    auto expr = subExpression->expression(table, vids);
    if (expr != nullptr)
      subExprs.push_back(expr);
  }

  SimpleExpression* last = nullptr;
  for (const auto& subExpr : subExprs) {
    if (dynamic_cast<TrueExpression*>(subExpr) == nullptr) {
      if (last == nullptr)
        last = subExpr;
      else
        last = new CompoundExpression(last, subExpr, AND);
    }
    else {
      delete subExpr;
    }
  }

  if (last == nullptr)
    return new TrueExpression();

  return last;
}

void AndExpression::verify() const {
  for (const auto& subExpression : _subExpressions)
    subExpression->verify();
}

std::shared_ptr<AndExpression> AndExpression::parse(const Json::Value& data) {
  std::vector<std::shared_ptr<SelectExpression>> subs;
  if (data.isMember("sub")) {
    const auto sub = data["sub"];
    for (unsigned i = 0; i < sub.size(); ++i)
      subs.push_back(SelectExpression::parse(sub[i]));
  }
  
  return std::make_shared<AndExpression>(subs);
}

bool AndExpression::accessesTable(storage::atable_ptr_t table) const {
  for (const auto& subExpression : _subExpressions) {
    if (subExpression->accessesTable(table))
      return true;
  }
  return false;
}

std::vector<std::string> AndExpression::accessedTables() const {
  std::vector<std::string> ret;
  for (const auto& subExpression : _subExpressions) {
    const auto& tables = subExpression->accessedTables();
    for (const auto& table : tables)
      ret.push_back(table);
  }
  const auto& newEnd = std::unique(ret.begin(), ret.end());
  ret.erase(newEnd, ret.end());
  return ret;
}

std::vector<std::string> AndExpression::accessedFields(const std::string& table) const {
  std::vector<std::string> ret;
  for (const auto& subExpression : _subExpressions) {
    const auto& fields = subExpression->accessedFields(table);
    for (const auto& field : fields)
      ret.push_back(field);
  }
  const auto& newEnd = std::unique(ret.begin(), ret.end());
  ret.erase(newEnd, ret.end());
  return ret;
}

} } } // namespace hyrise::access::aging

