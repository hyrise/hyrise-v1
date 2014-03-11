// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>
#include <string>
#include <jsoncpp/json.h>

#include <access/expressions/pred_SimpleExpression.h>

namespace hyrise {
namespace access {
namespace aging {

class SelectExpression {
public:
  virtual ~SelectExpression() {}

  virtual SimpleExpression* expression(storage::atable_ptr_t table,
                                       const std::map<storage::field_t, std::vector<storage::value_id_t>>& vids) const = 0;
  virtual void verify() const = 0;

  static std::shared_ptr<SelectExpression> parse(const Json::Value& data);

  virtual bool accessesTable(storage::atable_ptr_t table) const = 0;

  virtual std::vector<std::string> accessedTables() const = 0;
  virtual std::vector<std::string> accessedFields(const std::string& table) const = 0;
};

} } } // namespace hyrise::access::aging

