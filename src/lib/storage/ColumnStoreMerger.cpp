// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "storage/ColumnStoreMerger.h"

#include <cassert>

#include "storage/AbstractMerger.h"
#include "storage/ColumnMetadata.h"
#include "storage/Table.h"
#include "storage/MutableVerticalTable.h"
#include "storage/OrderPreservingDictionary.h"
#include "storage/ConcurrentUnorderedDictionary.h"
#include "storage/ConcurrentFixedLengthVector.h"
#include "storage/BitCompressedVector.h"

#include "storage/DictionaryFactory.h"
#include "storage/Table.h"

#include "storage/meta_storage.h"
#include "storage/GroupkeyIndex.h"

#include "io/StorageManager.h"

namespace hyrise {
namespace storage {

struct MergeColumnFunctor {
  typedef void value_type;
  size_t column;
  ColumnStoreMerger& csm;

  MergeColumnFunctor(size_t c, ColumnStoreMerger& csm) : column(c), csm(csm) {}

  template <typename R>
  value_type operator()() {
    csm.mergeDictionary<R>(column);
  }
};

namespace {

auto create_concurrent_dict = [](DataType dt) { return makeDictionary(types::getConcurrentType(dt)); };
auto create_concurrent_storage = [](std::size_t cols) {
  return std::make_shared<ConcurrentFixedLengthVector<value_id_t>>(cols, 0);
};
}

// forceFullIndexRebuild can be passed to the corresponding PlanOp or the Constructor of this class to prevent
// the merger from calling "recreateIndexMergeDict" and create all indices from scratch.
ColumnStoreMerger::ColumnStoreMerger(std::shared_ptr<Store> store, bool forceFullIndexRebuild)
    : _store(store),
      _main(store->getMainTable()),
      _delta(store->getDeltaTable()),
      _main_indices(store->getMainIndices()),
      _newMainSize(store->size()),
      _columnCount(_store->columnCount()),
      _forceFullIndexRebuild(forceFullIndexRebuild) {
  _vidMappingDelta.resize(_delta->size());
  _newTables.reserve(_columnCount);
};

template <typename T>
void ColumnStoreMerger::mergeDictionary(size_t column) {
  // This method performs the actual merge according to section four of the paper
  // 'Fast Lookups for In-Memory Column Stores'. You can find it at:
  // http://ares.epic.hpi.uni-potsdam.de/apps/static/papers/Fast_Lookups_for_In-Memory_Column_Stores_latest.pdf
  // If the column which should be merged is indexed the algorithm tries to call the method "recreateIndexMergeDict"
  // which will handle the merging of dictionaries, attribute vectors and index structures, if the method is not
  // implemented it will simply merge dictionaries and attribute vectors as if there were no indices and rebuild the
  // indices from scratch, if there are any, in the end.

  std::shared_ptr<AbstractDictionary> newDict = nullptr;
  std::shared_ptr<AbstractIndex> newIndex = nullptr;

  // execute merge and rebuild index from scratch if there is one for this particular column if:
  // - forceFullIndexRebuild flag is set
  // - no more indices are set for this table
  // - there are more main indices, but not for this column
  // - there is an index, but the recreateIndexMergeDict is not implemented -> rebuild from scracht
  if (_forceFullIndexRebuild || _main_indices.size() < _currentIndexToMerge + 1 ||
      _main_indices[_currentIndexToMerge].second[0] != column ||
      (newIndex = _main_indices[_currentIndexToMerge].first->recreateIndexMergeDict(
           column, _store, newDict, _vidMappingMain, _vidMappingDelta)) == nullptr) {
    auto mainDictionary =
        std::dynamic_pointer_cast<storage::OrderPreservingDictionary<T>>(_main->dictionaryAt(column, 0, 0));

    auto deltaVector = std::dynamic_pointer_cast<storage::ConcurrentFixedLengthVector<value_id_t>>(
        _delta->getAttributeVectors(column)[0].attribute_vector);
    auto deltaDictionary =
        std::dynamic_pointer_cast<storage::ConcurrentUnorderedDictionary<T>>(_delta->dictionaryAt(column, 0, 0));

    auto newDictNoIndex = std::make_shared<OrderPreservingDictionary<T>>(deltaDictionary->size());

    auto it = mainDictionary->begin();
    auto itDelta = deltaDictionary->begin();
    auto deltaEnd = deltaDictionary->end();

    size_t mainDictSize = mainDictionary->size();

    _vidMappingMain.reserve(mainDictSize);

    T mainDictValue;

    ValueId vid;

    uint64_t m = 0, n = 0;

    // mapping of values to positions in deltaVector necessary to be able to build help structure for merging
    // (_vidMappingDelta)
    // efficiently
    std::map<T, std::vector<size_t>> deltaValueMap;
    for (size_t j = 0; j < deltaVector->size(); ++j) {
      deltaValueMap[deltaDictionary->getValueForValueId(deltaVector->getRef(0, j))].push_back(j);
    }

    while (itDelta != deltaEnd || m != mainDictSize) {
      if (m != mainDictSize)
        mainDictValue = mainDictionary->getValueForValueId(m);

      bool processM = (itDelta == deltaEnd || (m != mainDictSize && mainDictValue <= *itDelta));
      bool processD = (m == mainDictSize || *itDelta <= mainDictValue);

      if (processM) {
        vid.valueId = newDictNoIndex->addValue(mainDictValue);

        _vidMappingMain.push_back(n - m);
        ++m;
      }

      if (processD) {
        if (!processM)
          vid.valueId = newDictNoIndex->addValue(*itDelta);

        for (auto it = deltaValueMap[*itDelta].cbegin(); it != deltaValueMap[*itDelta].cend(); ++it)
          _vidMappingDelta[*it] = n;

        ++itDelta;
      }
      ++n;
    }

    newDict = newDictNoIndex;
  }

  std::shared_ptr<Table> table;

  // Poor man's attribute vector type switch
  if (std::dynamic_pointer_cast<FixedLengthVector<value_id_t>>(
          _main->getAttributeVectors(column)[0].attribute_vector)) {
    table = std::make_shared<Table>(std::vector<ColumnMetadata>{_main->metadataAt(column)},
                                    std::make_shared<FixedLengthVector<value_id_t>>(1, 0),
                                    std::vector<adict_ptr_t>{newDict});
  } else if (std::dynamic_pointer_cast<BitCompressedVector<value_id_t>>(
                 _main->getAttributeVectors(column)[0].attribute_vector)) {
    table = std::make_shared<Table>(std::vector<ColumnMetadata>{_main->metadataAt(column)},
                                    std::make_shared<BitCompressedVector<value_id_t>>(1, 0),
                                    std::vector<adict_ptr_t>{newDict});
  } else {
    throw std::runtime_error("Unsupported attribute vector type in ColumnStoreMerge");
  }

  _newTables.push_back(table);

  table->resize(_newMainSize);

  mergeValues(column, table);

  // Index present, but recreateIndexMergeDict not implemented or a full rebuild is forced
  if (_currentIndexToMerge < _main_indices.size() && _main_indices[_currentIndexToMerge].second[0] == column &&
      newIndex == nullptr) {
    newIndex = _main_indices[_currentIndexToMerge].first->recreateIndex(table, 0);
  }

  if (newIndex != nullptr) {
    _store->addMainIndex(newIndex, field_list_t{column});
    io::StorageManager* sm = io::StorageManager::getInstance();
    sm->replace(_main_indices[_currentIndexToMerge++].first->getId(), newIndex);
  }
}

void ColumnStoreMerger::mergeValues(size_t column, atable_ptr_t table) {
  size_t mainTableSize = _main->size();
  std::shared_ptr<hyrise::storage::BaseAttributeVector<value_id_t>> mainVector =
      std::dynamic_pointer_cast<storage::BaseAttributeVector<value_id_t>>(
          _main->getAttributeVectors(column)[0].attribute_vector);

  value_id_t currentVid;
  for (size_t j = 0; j < mainTableSize; ++j) {
    currentVid = mainVector->get(0, j);
    table->setValueId(0, j, ValueId{currentVid + _vidMappingMain[currentVid], 0});
  }

  for (size_t j = mainTableSize; j < _newMainSize; ++j) {
    table->setValueId(0, j, ValueId{_vidMappingDelta[j - mainTableSize], 0});
  }
}

void ColumnStoreMerger::merge() {
  // This is the main method of the ColumnStoreMerger, it delivers some preparation work before
  // it triggers the actual merge. Afterwards it replaces the main and delta partition of the
  // store with the result of the merge, the new main and an empty delta partition.
  // The world is so kind to stop for merging which means that no insert or update operations
  // are possible while the merge is running. Read operations should still work.

  field_list_t temp_field_list;
  for (size_t i = 0; i < _columnCount; ++i)
    temp_field_list.push_back(i);

  _currentIndexToMerge = 0;

  _store->clearIndices();

  for (size_t i = 0; i < _columnCount; ++i) {
    MergeColumnFunctor fun(i, *this);
    storage::type_switch<hyrise_basic_types> ts;
    ts(_main->typeOfColumn(i), fun);
    _vidMappingMain.clear();
  }

  std::shared_ptr<storage::AbstractTable> newMain = std::make_shared<storage::MutableVerticalTable>(_newTables);

  atable_ptr_t newDelta = _delta->copy_structure(create_concurrent_dict, create_concurrent_storage);
  newDelta->setName(_store->getName());
  if (_store->loggingEnabled())
    newDelta->enableLogging();

  // Reader/Writer locks should be used here to prevent HYRISE from possible crashes. The world
  // would still stop of course
  _store->setDelta(newDelta);
  _store->setMain(newMain);
}
}
}  // namespace hyrise::storage
