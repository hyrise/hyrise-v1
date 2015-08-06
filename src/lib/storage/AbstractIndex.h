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
 protected:
  std::string _id = "AbstractIndex_0";

 public:
  AbstractIndex();

  AbstractIndex(std::string id);

  virtual ~AbstractIndex();

  virtual void shrink() = 0;

  virtual void write_lock() = 0;

  virtual void unlock() = 0;

  // Remove this method as soon as plan ops do not need names of indices anymore (the store knows it's indices)
  virtual std::string getId();

  virtual std::shared_ptr<AbstractIndex> recreateIndex(const c_atable_ptr_t& in, field_t column);

  virtual std::shared_ptr<AbstractIndex> recreateIndexMergeDict(size_t column,
                                                                std::shared_ptr<Store> _store,
                                                                std::shared_ptr<AbstractDictionary>& newDictReturn,
                                                                std::vector<value_id_t>& x,
                                                                std::vector<value_id_t>& Vd);
};
}
}  // namespace hyrise::storage
