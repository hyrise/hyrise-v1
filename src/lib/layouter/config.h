// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#define CACHE_LINE_SIZE 64
#define HYRISE_COST "HYRISECost"
#define ROW_COST "RowCost"
#define COL_COST "ColumnCost"

namespace hyrise {
namespace layouter {

struct LayouterConfiguration {
  typedef enum {
    access_type_fullprojection,
    access_type_outoforder,
    access_type_default
  } access_type_t;
};

} } // namespace hyrise::layouter
