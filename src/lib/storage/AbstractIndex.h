// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
/** @file AbstractIndex.h
 *
 * Contains class definition for InvertedIndex Base Class
 *
 */

#ifndef SRC_LIB_STORAGE_ABSTRACTINDEX_H_
#define SRC_LIB_STORAGE_ABSTRACTINDEX_H_

#include <storage/AbstractResource.h>

class AbstractIndex : public AbstractResource {

public:
  /**
   * Destructor.
   */
  virtual ~AbstractIndex();

  virtual void shrink() = 0;
};

#endif  // SRC_LIB_STORAGE_ABSTRACTINDEX_H_

