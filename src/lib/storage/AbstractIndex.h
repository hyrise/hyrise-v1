// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/** @file AbstractIndex.h
 *
 * Contains class definition for InvertedIndex Base Class
 *
 */
#pragma once

#include <storage/AbstractResource.h>

namespace hyrise {
namespace storage {

class AbstractIndex : public AbstractResource {

public:
  /**
   * Destructor.
   */
  virtual ~AbstractIndex();

  virtual void shrink() = 0;
};

} } // namespace hyrise::storage


