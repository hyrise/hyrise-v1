#pragma once

#include <list>
#include "storage/AbstractTable.h"
#include "storage/Table.h"

namespace hyrise {
namespace storage {

struct Part {
  std::size_t columnOffset, verticalStart, verticalEnd;
  Table const& table;
};

inline std::ostream& operator<<(std::ostream& os, const storage::Part& part) {
  return os << "Part column:" << part.columnOffset << " rows: [" << part.verticalStart << " " << part.verticalEnd
            << "]";
}


std::list<Part> column_parts_extract(const AbstractTable& tbl, std::size_t column, std::size_t start, std::size_t stop);
}
}
