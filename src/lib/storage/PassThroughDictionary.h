// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <type_traits>
#include <memory>

#include <storage/BaseDictionary.h>
#include <storage/BaseIterator.h>
#include <storage/DictionaryIterator.h>
#include <storage/storage_types.h>

namespace hyrise {
namespace storage {

template <typename T>
class PassThroughDictionaryIterator;


template <typename T>
class PassThroughDictionary : public BaseDictionary<T> {

  union pt_dict_union_t {
    const value_id_t vid;
    const T value;

    explicit pt_dict_union_t(const value_id_t v): vid(v){}
    explicit pt_dict_union_t(const T v): value(v){}

  };

  static_assert(sizeof(T) == sizeof(value_id_t), 
		     "PassThroughDictionary can only be used with types contained in value_id_t");

public:

  explicit PassThroughDictionary(size_t s = 0) {}

  value_id_t addValue(T value) { 
    return pt_dict_union_t(value).vid;
  }

  T getValueForValueId(value_id_t value_id) { 
    return pt_dict_union_t(value_id).value;
  }

  value_id_t getValueIdForValue(const T& value) const {
    return pt_dict_union_t(value).vid;
  }

  value_id_t getValueIdForValueSmaller(T other) {
    STORAGE_NOT_IMPLEMENTED(PassThroughDictionary, getValueIdForValueSmaller());
  }

  value_id_t getValueIdForValueGreater(T other) {
    STORAGE_NOT_IMPLEMENTED(PassThroughDictionary, getValueIdForValueGreater());
  }

  const T getSmallestValue() {
    STORAGE_NOT_IMPLEMENTED(PassThroughDictionary, getSmallestValue());
  }

  const T getGreatestValue() {
    STORAGE_NOT_IMPLEMENTED(PassThroughDictionary, getGreatestValue());
  }

  bool isValueIdValid(value_id_t value_id) { return true; };

  bool valueExists(const T& value) const { return true; };

  void reserve(size_t) {};

  size_t size() { return 0; }

  std::shared_ptr<AbstractDictionary> copy() {
    return copy_empty();
  }

  std::shared_ptr<AbstractDictionary> copy_empty() {
    return std::make_shared<PassThroughDictionary<T> >();
  }

  void shrink(){}

  bool isOrdered() { return false; }

  using iterator = DictionaryIterator<T>;

  DictionaryIterator<T> begin(){ return iterator(std::make_shared<PassThroughDictionaryIterator<T> >()); }
  DictionaryIterator<T> end() { return iterator(std::make_shared<PassThroughDictionaryIterator<T> >()); }
    
};

/*
 * The PassThrough Dictionary Iterator does nothing as it does nothing
 * and there is nothing to iterate on, it's just convenience to have
 * it and not use it :)
 */
template <typename T>
class PassThroughDictionaryIterator : public BaseIterator<T> {
  void increment() {}

  bool equal(const std::shared_ptr<BaseIterator<T>>& other) const {
    return true;
  }

  T &dereference() const {
    STORAGE_NOT_IMPLEMENTED(PassThroughDictionaryIterator, dereference());
  }

  value_id_t getValueId() const {
    STORAGE_NOT_IMPLEMENTED(PassThroughDictionaryIterator, getValueId());
  }
};


}}
