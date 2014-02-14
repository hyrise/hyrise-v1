// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "EqualExpression.h"

#include <iostream>

#include <storage/storage_types_helper.h>
#include <io/StorageManager.h>

#include <access/expressions/pred_InExpression.h>

namespace hyrise {
namespace access {
namespace aging {

EqualExpression::EqualExpression(const std::string& table, const std::string& field) :
    _table(table),
    _field(field) {}

namespace {

struct create_in_expr_functor {
  typedef void value_type;

  create_in_expr_functor(storage::atable_ptr_t table,
                         storage::field_t field,
                         const std::vector<storage::value_id_t>& vids) :
    _table(table),
    _field(field),
    _vids(vids) {}

  template <typename T>
  void operator()() {
    const auto& dict = checked_pointer_cast<storage::BaseDictionary<T>>(_table->dictionaryAt(_field));
    std::vector<T> values(_vids.size());
    for (unsigned i = 0; i < values.size(); ++i)
      values.at(i) = dict-> getValueForValueId(_vids.at(i));

    expr = new InExpression<T>(_table, _field, values);
  }

  SimpleExpression* expr;

 private:
  const storage::atable_ptr_t _table;
  const storage::field_t _field;
  const std::vector<storage::value_id_t> _vids;
};

} // namespace

SimpleExpression* EqualExpression::expression(storage::atable_ptr_t table,
                                              const std::map<storage::field_t, std::vector<storage::value_id_t>>& vids) const {
  /*TODO this is currently not working as the table is replaced
  if (!accessesTable(table)) {
    std::cout << "(query does not access table)" << std::endl;
    return nullptr;
  }*/

  const auto field = table->numberOfColumn(_field);

  if (vids.find(field) == vids.end()) {
    std::cout << "(no value ids for field)" << std::endl;
    return nullptr; //TODO really? not sure
  }

  create_in_expr_functor functor(table, field, vids.at(field));
  storage::type_switch<hyrise_basic_types> ts;
  ts(table->typeOfColumn(field), functor);

  return functor.expr;
}

void EqualExpression::verify() const {
  const auto& sm = *io::StorageManager::getInstance();
  const auto& table = sm.get<storage::AbstractTable>(_table);
  table->numberOfColumn(_field);
}

std::shared_ptr<EqualExpression> EqualExpression::parse(const Json::Value& data) {
  if (!data.isMember("table"))
    throw std::runtime_error("a table needs to be specified for an expression");
  if (!data.isMember("field"))
    throw std::runtime_error("a field needs to be specified for an expression");

  const auto& table = data["table"].asString();
  const auto& field = data["field"].asString();

  return std::make_shared<EqualExpression>(table, field);
}

bool EqualExpression::accessesTable(storage::atable_ptr_t table) const {
  const auto& sm = *io::StorageManager::getInstance();
  return sm.get<storage::AbstractTable>(_table) == table;
}

} } } // namespace aging::hyrise::access

