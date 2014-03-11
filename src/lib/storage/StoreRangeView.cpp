#include "StoreRangeView.h"

#include <storage/TableRangeView.h>

namespace hyrise {
namespace storage {

StoreRangeView::StoreRangeView(store_ptr_t store, size_t size, size_t offset) :
  Store(std::make_shared<TableRangeView>(store->getMainTable(), offset, offset + size),
        store->getDeltaTable()) {}

} } // namespace hyrise::storage
