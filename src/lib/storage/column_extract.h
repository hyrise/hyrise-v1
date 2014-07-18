#pragma once

#include <list>
#include "storage/AbstractTable.h"
#include "storage/Table.h"

namespace hyrise {
namespace storage {

    struct Part {
        std::size_t columnOffset, verticalStart, verticalEnd;
        Table const & table;
    };

    std::list<Part> column_parts_extract(const AbstractTable& tbl, std::size_t column);

}}
