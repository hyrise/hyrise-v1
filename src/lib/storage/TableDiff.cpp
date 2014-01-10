// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "TableDiff.h"

#include <iostream>
#include <storage/AbstractTable.h>
#include <storage/meta_storage.h>

namespace hyrise { namespace storage {
namespace{
/// Functor class for usage with hyrise::storage::type_switch to compare table cells by value

type_switch<hyrise_basic_types> ts;

class EqualValueFunctor {
private:
  const AbstractTable* const _left;
  const AbstractTable* const _right;
  const size_t &_line_left, _line_right;
  const size_t &_col_left, _col_right;
public:
  typedef bool value_type;
  EqualValueFunctor(const AbstractTable* const left,
                    const AbstractTable* const right,
                    const size_t &line_left, const size_t &line_right,
                    const field_t &col_left, const field_t &col_right) :
    _left(left), _right(right),
    _line_left(line_left), _line_right(line_right),
    _col_left(col_left), _col_right(col_right)
  {}

  template <typename R>
  bool operator()() {
    return _left->getValue<R>(_col_left, _line_left) == _right->getValue<R>(_col_right, _line_right);
  }
};

bool line_equal(const AbstractTable* const left,
                           const AbstractTable* const right,
                           const size_t line_left, const size_t line_right,
                           std::map<field_t, field_t> mapFields) {
  for (field_t col_left = 0; col_left < left->columnCount(); ++col_left) {
    std::string column_name = left->nameOfColumn(col_left);
    field_t col_right;
    if (mapFields.find(col_left) == mapFields.end())
      continue;
    col_right = mapFields[col_left];
    EqualValueFunctor ev(left, right,
                         line_left, line_right,
                         col_left, col_right);
    bool equal = ts(left->typeOfColumn(col_left), ev);
    if (!equal)
      return false;
  }
  return true;
}

std::map<field_t, field_t> compareSchemas(const AbstractTable* const left,
                                                     const AbstractTable* const right) {
  std::map<field_t, field_t> m;
  for (field_t col_left = 0; col_left < left->columnCount(); col_left++) {
    try{
      field_t col_right = right->numberOfColumn(left->nameOfColumn(col_left));
        m[col_left] = col_right;
    }
    catch (MissingColumnException exception) {}
  }

  return m;
}
} //namespace


bool TableDiff::compare(const TableDiff::RelationEqType eqType) const {
  switch (eqType) {
    case TableDiff::RelationEq: return equal();
    case TableDiff::RelationEqSorted: return equalSorted();
  }
  return false; //connot happen but there is a compiler warning
}
bool TableDiff::equal() const {
  return rowsMatched() && schemasMatched();
}
bool TableDiff::equalSorted() const {
  return rowsMatchedSorted() && schemasMatched();
}

bool TableDiff::schemasMatched() const {
  for (auto i = fields.begin(); i != fields.end(); i++)
    if (*i != TableDiff::FieldCorrect)
      return false;
  return true;
}

bool TableDiff::rowsMatchedSorted() const {
  return wrongRows.empty() && falsePositionRows.empty();
}
bool TableDiff::rowsMatched() const {
  return wrongRows.empty();
}


TableDiff TableDiff::diffTables(const AbstractTable* const left,
                                const AbstractTable* const right,
                                const bool schema_only) {
  TableDiff diff;

  std::vector<bool>
            seen_right(right->size(), false);

  auto mapFields = compareSchemas(left ,right);

  //compare relation scheme
  diff.fields.resize(left->columnCount(), TableDiff::FieldWrong);
  for (auto i = mapFields.begin(); i != mapFields.end(); ++i) {
    if (types::isCompatible(right->typeOfColumn(i->second),left->typeOfColumn(i->first)))
      diff.fields[i->first] = TableDiff::FieldCorrect;
    else
      diff.fields[i->first] = TableDiff::FieldWrongType;
  }
  //remove wrong typed fields from map
  for (size_t i = 0; i < diff.fields.size(); ++i)
    if (diff.fields[i] != FieldCorrect)
      mapFields.erase(i);

  if (!schema_only) {
    //compare rows
    for (size_t line_left = 0; line_left < left->size(); ++line_left) {
      bool foundMatch = false;
      for (size_t line_right = 0; line_right < right->size(); ++line_right) {
        // let's not check lines that have already been marked as seen
        if (seen_right[line_right])
          continue;

        if (line_equal(left, right, line_left, line_right, mapFields)) {
          seen_right[line_right] = true;
          if (line_left != line_right)
            diff.falsePositionRows[line_left]=line_right;
          foundMatch = true;
          break;
        }
      }
      if (!foundMatch) {
        diff.wrongRows.push_back(line_left);
      }
    }
  }
  return diff;
}

} } // namespace hyrise::storage

