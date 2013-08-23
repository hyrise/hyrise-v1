// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_STORAGE_TABLEDIFF_H_
#define SRC_LIB_STORAGE_TABLEDIFF_H_

#include <vector>
#include <map>
#include <memory>

#include <storage/storage_types.h>

class AbstractTable;

namespace hyrise { namespace storage {

class TableDiff {

public:
  enum RelationEqType {RelationEq, RelationEqSorted};
  enum FieldCorrectness {FieldCorrect, FieldWrongType, FieldWrong};

  TableDiff() {}
  ~TableDiff() {}

  bool compare(const RelationEqType eqType = RelationEq) const;
  bool equal() const;
  bool equalSorted() const;
  bool schemasMatched() const;
  bool rowsMatched() const;
  bool rowsMatchedSorted() const;

  static TableDiff diffTables(const AbstractTable* const left,
                              const AbstractTable* const right,
                              const bool schema_only = true);


  std::vector<FieldCorrectness> fields;
  std::vector<size_t> wrongRows;
  std::map<size_t,size_t> falsePositionRows;
};

}}

#endif //SRC_LIB_STORAGE_TABLEDIFF_H_
