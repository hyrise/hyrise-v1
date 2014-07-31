#include "storage/debug_table_structure.h"

#include "storage/PointerCalculator.h"
#include "storage/AbstractTable.h"
#include "storage/MutableVerticalTable.h"
#include "storage/HorizontalTable.h"
#include "storage/Store.h"

#include "helper/demangle.h"

namespace hyrise {
namespace storage {

#define VISIT2(T)                        \
  Visitation visitEnter(T& t) override { \
    level += 1;                          \
    X::handle(t, level);                 \
    return Visitation::next;             \
  }                                      \
  Visitation visitLeave(T&) override {   \
    level -= 1;                          \
    return Visitation::next;             \
  }                                      \
  Visitation visit(T& t) override {      \
    X::handle(t, level);                 \
    return Visitation::next;             \
  }

template <typename X>
class HierarchyVisitor : public StorageVisitor {
 public:
  VISIT2(const Store);
  VISIT2(const Table);
  VISIT2(const MutableVerticalTable);
  VISIT2(const HorizontalTable);
  VISIT2(const hyrise::storage::PointerCalculator);

  std::size_t level = 0;
};

#undef VISIT

class DebugStructure : public HierarchyVisitor<DebugStructure> {
 public:
  template <typename T>
  static void handle(const T& t, size_t level) {
    std::cout << std::string(level, '\t') << demangle(typeid(t).name()) << std::endl;
  }

  static void handle(const Table& t, size_t level) {
    level += 1;
    std::cout << std::string(level, '\t') << demangle(typeid(t).name()) << std::endl;
    std::cout << std::string(level, '\t') << demangle(typeid(*t.dictionaryAt(0).get()).name())
              << " ordered:" << t.dictionaryAt(0)->isOrdered() << std::endl;
    std::cout << std::string(level, '\t')
              << demangle(typeid(*t.getAttributeVectors(0).at(0).attribute_vector.get()).name()) << std::endl;
  }
};

void debug_structure(const c_atable_ptr_t& table) {
  DebugStructure ds;
  table->accept(ds);
}
}
}
