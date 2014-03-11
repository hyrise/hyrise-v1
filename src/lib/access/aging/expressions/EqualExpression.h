// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <access/aging/expressions/SelectExpression.h>

namespace hyrise {
namespace access {
namespace aging {

class EqualExpression : public SelectExpression {
public:
  EqualExpression(const std::string& table, const std::string& field);
  virtual ~EqualExpression() {}

  virtual SimpleExpression* expression(storage::atable_ptr_t table,
                                       const std::map<storage::field_t, std::vector<storage::value_id_t>>& vids) const;
  virtual void verify() const;

  static std::shared_ptr<EqualExpression> parse(const Json::Value& data);

  virtual bool accessesTable(storage::atable_ptr_t table) const;

  virtual std::vector<std::string> accessedTables() const;
  virtual std::vector<std::string> accessedFields(const std::string& table) const;

  virtual std::vector<storage::value_id_t> vids(const std::string& tableName, const storage::c_atable_ptr_t& table,
                                               const std::string& field) const;

private:
  const std::string _table;
  const std::string _field;
};

} } } // namespace aging::hyrise::access

