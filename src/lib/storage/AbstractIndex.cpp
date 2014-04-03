// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <storage/AbstractIndex.h>

namespace hyrise {
namespace storage {

AbstractIndex::~AbstractIndex() {}

void AbstractIndex::recreateIndex(atable_ptr_t newMain) {
	// TODO: implement fallback
	;
}

bool AbstractIndex::recreateIndexMergeDict(size_t i, atable_ptr_t oldMain, atable_ptr_t oldDelta, atable_ptr_t newMain, std::shared_ptr<hyrise::storage::AbstractDictionary> newDict, std::vector<std::vector<value_id_t>> &x, value_id_t* VdColumn) {
	// TODO: implement fallback
	return false;
}

}
}  // namespace hyrise::storage
