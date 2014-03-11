// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <access/aging/expressions/SelectExpression.h>

namespace hyrise {
namespace access {
namespace aging {

class AndExpression : public SelectExpression {
public:
  AndExpression(const std::vector<std::shared_ptr<SelectExpression>>& subExpressions);
  virtual ~AndExpression() {}

  virtual SimpleExpression* expression(storage::atable_ptr_t table,
                                       const std::map<storage::field_t, std::vector<storage::value_id_t>>& vids) const;
  virtual void verify() const;

  static std::shared_ptr<AndExpression> parse(const Json::Value& data);

  virtual bool accessesTable(storage::atable_ptr_t table) const;

  virtual std::vector<std::string> accessedTables() const;
  virtual std::vector<std::string> accessedFields(const std::string& table) const;

private:
  std::vector<std::shared_ptr<SelectExpression>> _subExpressions;
};

} } } // namespace hyrise::access::aging

