// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>
#include <stdexcept>
#include <storage/AbstractAttributeVector.h>


namespace hyrise {
namespace storage {

/*
* This is the base class of the Attribute Vector that is implement in HYRISE.
* All sub-classes must implement the virtual methods.
*/
template <typename T>
class BaseAttributeVector : public AbstractAttributeVector {
public:
  typedef T value_type;

  virtual ~BaseAttributeVector() {
  }

  /*
  * Get a single value identified by column and row
  * @param size_t column Column in the vector
  * @param size_t row position offset
  */
  virtual T get(size_t column, size_t row) const = 0;

  /*
  * Set the value identified by column and row
  */
  virtual void set(size_t column, size_t row, T value) = 0;

  /*
   * Reseve memory for this container.
   */
  virtual void reserve(size_t rows) = 0;

  /*
   * Resize the given container to a size that is larger than the
   * current one. This will allocate all necessary size and set the
   * size of the container.
   */
  virtual void resize(size_t rows) = 0;

  virtual uint64_t capacity() = 0;

  virtual void clear() = 0;

  virtual size_t size() = 0;

  virtual std::shared_ptr<BaseAttributeVector<T>> copy() = 0;

  virtual void rewriteColumn(const size_t column, const size_t bits) = 0;

};

} } // namespace hyrise::storage

