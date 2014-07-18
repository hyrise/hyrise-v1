#pragma once

#include "helper/types.h"


namespace hyrise {
namespace storage {

#define VISIT(T)                                                 \
  virtual Visitation visitEnter(T&) { return Visitation::next; } \
  virtual Visitation visitLeave(T&) { return Visitation::next; } \
  virtual Visitation visit(T&) { return Visitation::next; }

enum class Visitation : bool {
  next,
  skip
};

class MutableStorageVisitor {
 public:
  virtual ~MutableStorageVisitor();
  VISIT(MutableVerticalTable);
  VISIT(Store);
  VISIT(Table);
  VISIT(HorizontalTable);
  VISIT(PointerCalculator);
};

class StorageVisitor {
 public:
  virtual ~StorageVisitor();
  VISIT(const MutableVerticalTable);
  VISIT(const Store);
  VISIT(const Table);
  VISIT(const HorizontalTable);

  VISIT(const PointerCalculator);
};

#undef VISIT
}
}
