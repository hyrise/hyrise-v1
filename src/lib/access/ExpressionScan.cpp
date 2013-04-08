// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/ExpressionScan.h"

#include "access/default_strategy.h"
#include "access/QueryParser.h"

#include "storage/MutableVerticalTable.h"
#include "storage/Table.h"

namespace hyrise {
namespace access {

ColumnExpression::ColumnExpression(const storage::atable_ptr_t &t) : _table(t) {
}

ColumnExpression::~ColumnExpression() {
}

AddExp::AddExp(const storage::atable_ptr_t &t,
               const storage::field_t field1,
               const storage::field_t field2) : ColumnExpression(t), _field1(field1), _field2(field2) {
}

AddExp::~AddExp() {
}

void AddExp::setResult(const storage::atable_ptr_t &result,
                       const storage::field_t column,
                       const storage::pos_t row) const {
  result->setValue<hyrise_int_t>(column, row, _table->getValue<hyrise_int_t>(_field1, row) + _table->getValue<hyrise_int_t>(_field2, row));
}

std::string AddExp::getName() const {
  return _table->nameOfColumn(_field1) + "+" + _table->nameOfColumn(_field2);
}

DataType AddExp::getType() const {
  return IntegerType;
}

ExpressionScan::~ExpressionScan() {
}

void ExpressionScan::executePlanOperation() {
  size_t input_size = input.getTable(0)->size();

  metadata_list metadata;
  ColumnMetadata *m = new ColumnMetadata(_column_name, _expression->getType());
  metadata.push_back(m);

  std::vector<AbstractTable::SharedDictionaryPtr> dicts;
  dicts.push_back(AbstractDictionary::dictionaryWithType<DictionaryFactory<OrderIndifferentDictionary>>(_expression->getType()));

  storage::atable_ptr_t exp_result = std::make_shared<Table<DEFAULT_STRATEGY>>(&metadata, &dicts, 0, false);
  exp_result->resize(input_size);

  for (size_t row = 0; row < input_size; ++row) {
    /// Execute the predicate on the list
    _expression->setResult(exp_result, 0, row);
  }

  std::vector<storage::atable_ptr_t> vc;
  vc.push_back(std::const_pointer_cast<AbstractTable>(input.getTable(0)));
  vc.push_back(exp_result);

  addResult(std::make_shared<const MutableVerticalTable>(vc));
}

const std::string ExpressionScan::vname() {
  return "ExpressionScan";
}

void ExpressionScan::setExpression(const std::string &name,
                                   ColumnExpression *expression) {
  _expression = expression;
  _column_name = name;
}

}
}
