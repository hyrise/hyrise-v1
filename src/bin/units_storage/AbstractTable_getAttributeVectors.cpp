// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <list>
#include <stack>

#include "testing/test.h"
#include "io/shortcuts.h"
#include "storage/AbstractTable.h"

#include "storage/Table.h"
#include "storage/MutableVerticalTable.h"

namespace hyrise {
namespace storage {

class TableVisitor : public StorageVisitor {
 public:
  Visitation visit(const Table& t) {
    std::cout << &t << std::endl;
    return Visitation::next;  // continue visitation
  }
};

class ColumnPartsCollector : public StorageVisitor {
 public:
  ColumnPartsCollector(std::uint16_t column) : column(column) {}
  std::list<std::tuple<const Table&, std::uint16_t> >&& getParts() { return std::move(parts); }

 private:
  Visitation visitEnter(const MutableVerticalTable& t) override {
    offsets.push(verticalOffset);
    return Visitation::next;
  }

  Visitation visitLeave(const MutableVerticalTable& t) override {
    verticalOffset = offsets.top();
    offsets.pop();
    return Visitation::next;
  }

  Visitation visit(const Table& t) override {
    auto tableOffset = t.columnCount();
    if (verticalOffset + tableOffset > column) {
      parts.emplace_back(t, column - verticalOffset);
      return Visitation::skip;
    } else {
      verticalOffset += tableOffset;
      return Visitation::next;
    }
  }

  std::uint16_t column;
  std::uint16_t verticalOffset = 0;
  std::stack<std::uint16_t> offsets;
  std::list<std::tuple<const Table&, std::uint16_t> > parts;
};



TEST(AbstractTable_getAttributeVectors, test) {
  const auto& table = io::Loader::shortcuts::load("test/tables/employees.tbl");
  const auto& vectors = table->getAttributeVectors(1);
  EXPECT_EQ(2u, vectors.size());
}

TEST(AbstractTable_getAttributeVectors, test2) {
  const auto& table = io::Loader::shortcuts::load("test/tables/employees.tbl");
  const auto& vectors = table->allParts();
  EXPECT_EQ(2u, vectors.size());
}

TEST(AbstractTable_getAttributeVectors, test3 ) {
  const auto& table = io::Loader::shortcuts::load("test/partitioned_test.tbl");
  TableVisitor dv;
  table->accept(dv);
  ColumnPartsCollector cp(2);
  table->accept(cp);
  ColumnPartsCollector cp2(1);
  table->accept(cp2);
  auto parts = cp2.getParts();
}

}
}  // namespace hyrise::storage
