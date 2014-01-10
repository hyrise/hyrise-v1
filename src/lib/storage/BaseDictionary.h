// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <limits>

#include <storage/storage_types.h>
#include <storage/AbstractDictionary.h>
#include <storage/DictionaryIterator.h>

namespace hyrise {
namespace storage {

template <typename T>
class BaseDictionary : public AbstractDictionary {
public:
  using value_type = T;
  virtual value_id_t addValue(T value) = 0;

  virtual T getValueForValueId(value_id_t value_id) = 0;
  virtual value_id_t getValueIdForValue(const T &value) const = 0;

  /*
   * Returns the value id of the first value that is smaller
   * than other.
   *
   * @note needs to be implemented in each subclass
   * @param other the value to compare with
   */
  virtual value_id_t getValueIdForValueSmaller(T other) = 0;
  virtual value_id_t getValueIdForValueGreater(T other) = 0;

  virtual const T getSmallestValue() = 0;
  virtual const T getGreatestValue() = 0;

  virtual bool isValueIdValid(value_id_t value_id) = 0;
  virtual bool valueExists(const T &value) const = 0;

  virtual void reserve(size_t size) = 0;
  virtual size_t size() = 0;

  virtual std::shared_ptr<AbstractDictionary> copy() = 0;

  virtual bool isOrdered() = 0;

  typedef DictionaryIterator<T> iterator;

  virtual DictionaryIterator<T> begin() = 0;
  virtual DictionaryIterator<T> end() = 0;

  // Inserts a value into dictionary
  value_id_t insert(const T& value) {
    return getValueId(value, true);
  }

  // Retrieves value_id for given value, optionally creates
  // a new dictionary entry
  value_id_t getValueId(const T& value, bool create) {
    if (valueExists(value)) {
      return getValueIdForValue(value);
    } else if (create) {
      return addValue(value);
    } else {
      // TODO: We should document that INT_MAX is an invalid document ID
      return std::numeric_limits<value_id_t>::max();
    }
  }
};

} } // namespace hyrise::storage
