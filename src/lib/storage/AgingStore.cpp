// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <storage/AgingStore.h>
#include <iostream>

#include <io/TransactionManager.h>
#include <storage/storage_types.h>
#include <storage/PrettyPrinter.h>

#include <helper/vector_helpers.h>
#include <helper/locking.h>
#include <helper/cas.h>

#include "storage/DictionaryFactory.h"
#include "storage/ConcurrentUnorderedDictionary.h"
#include "storage/ConcurrentFixedLengthVector.h"

namespace hyrise { namespace storage {

AgingStore::AgingStore() :
  Store() {}

AgingStore::AgingStore(atable_ptr_t main_table) :
    Store(main_table) {}

AgingStore::~AgingStore() {}

} } // namespace hyrise::storage

