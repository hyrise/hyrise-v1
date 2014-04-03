// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/** @file AbstractIndex.h
 *
 * Contains class definition for InvertedIndex Base Class
 *
 */
#pragma once

#include <storage/AbstractResource.h>
#include <helper/types.h>

namespace hyrise {
namespace storage {

class AbstractIndex : public AbstractResource {

 public:
  /**
   * Destructor.
   */
  virtual ~AbstractIndex();

  virtual void shrink() = 0;

  virtual void write_lock() = 0;

  virtual void unlock() = 0;

  virtual void recreateIndex(atable_ptr_t newMain);

  virtual
  bool recreateIndexMergeDict(size_t i, atable_ptr_t oldMain, atable_ptr_t oldDelta, atable_ptr_t newMain, std::shared_ptr<hyrise::storage::AbstractDictionary> newDict, std::vector<std::vector<value_id_t>> &x, value_id_t* VdColumn);
};
}
}  // namespace hyrise::storage
