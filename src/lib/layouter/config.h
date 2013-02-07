#ifndef SRC_LIB_LAYOUTER_CONFIG_H_
#define SRC_LIB_LAYOUTER_CONFIG_H_

#define CACHE_LINE_SIZE 64
#define HYRISE_COST "HYRISECost"
#define ROW_COST "RowCost"
#define COL_COST "ColumnCost"

namespace layouter {
struct LayouterConfiguration {
  typedef enum {
    access_type_fullprojection,
    access_type_outoforder,
    access_type_default
  } access_type_t;
};
}
#endif  // SRC_LIB_LAYOUTER_CONFIG_H_
