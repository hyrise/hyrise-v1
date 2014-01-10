// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <cstdint>
#include <string>
#include <limits>
#include <memory>
#include <vector>

namespace hyrise { namespace storage {
class AbstractResource;
class AbstractTable;
class AbstractIndex;
class AbstractHashTable;
class AbstractDictionary;
} } // namespace hyrise::storage


class PointerCalculator;

namespace hyrise {

namespace tx {

// FIXME: TX Count is limited to 2^63-1 instead of 2^64-1
typedef int64_t transaction_id_t;
typedef int64_t transaction_cid_t;

static const transaction_id_t MERGE_TID = 1;
	// the merge needs its own TID so that it is isolated from the other transactions
static const transaction_id_t START_TID = 2;
static const transaction_id_t MAX_TID = std::numeric_limits<transaction_id_t>::max();

static const transaction_id_t UNKNOWN = 0;
static const transaction_cid_t UNKNOWN_CID = 0;
static const transaction_cid_t INF_CID = std::numeric_limits<transaction_cid_t>::max();

enum class TX_CODE {
	TX_OK,
	TX_FAIL_CONCURRENT_COMMIT,
	TX_FAIL_OTHER
};

}

namespace access {
class AbstractExpression;
typedef std::unique_ptr<AbstractExpression> expression_uptr_t;
}

namespace storage {
class SimpleStore;
class MutableVerticalTable;
class Store;

typedef std::shared_ptr<AbstractResource> aresource_ptr_t;
typedef std::shared_ptr<const AbstractResource> c_aresource_ptr_t;

typedef std::shared_ptr<AbstractDictionary> adict_ptr_t;
typedef std::shared_ptr<const AbstractDictionary> c_adict_ptr_t;

typedef std::shared_ptr<AbstractTable> atable_ptr_t;
typedef std::shared_ptr<const AbstractTable> c_atable_ptr_t;

typedef std::shared_ptr<AbstractIndex> aindex_ptr_t;
typedef std::shared_ptr<const AbstractIndex> c_aindex_ptr_t;

typedef std::shared_ptr<AbstractHashTable> ahashtable_ptr_t;
typedef std::shared_ptr<const AbstractHashTable> c_ahashtable_ptr_t;

typedef std::shared_ptr<MutableVerticalTable> vtable_ptr_t;
typedef std::shared_ptr<const MutableVerticalTable> c_vtable_ptr_t;

typedef std::shared_ptr<Store> store_ptr_t;
typedef std::shared_ptr<const Store> c_store_ptr_t;

typedef std::shared_ptr<PointerCalculator> calc_ptr_t;
typedef std::shared_ptr<const PointerCalculator> c_calc_ptr_t;

typedef std::shared_ptr<SimpleStore> simplestore_ptr_t;
typedef std::shared_ptr<const SimpleStore> c_simplestore_ptr_t;

typedef int64_t hyrise_int_t;
typedef int32_t hyrise_int32_t;
typedef float hyrise_float_t;
typedef std::string hyrise_string_t;

typedef uint32_t value_id_t;
typedef unsigned char table_id_t;

typedef size_t pos_t;
typedef size_t field_t;

typedef std::string field_name_t;
typedef std::vector<field_name_t> field_name_list_t;

typedef std::vector<pos_t> pos_list_t;
typedef std::vector<field_t> field_list_t;
}

namespace taskscheduler {
  class Task;
  typedef std::shared_ptr<Task> task_ptr_t;
}

// constraints
static_assert(std::is_same<tx::transaction_id_t, storage::hyrise_int_t>::value,
              "transaction_id_t and hyrise_int_t need to be of the same type");
}
