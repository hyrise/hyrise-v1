// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/system/OperationData-Impl.h"

#include <string>

#include "storage/AbstractTable.h"
#include "storage/HashTable.h"

namespace hyrise {
namespace access {

void OperationData::addResource(const storage::c_aresource_ptr_t& resource) {
  _resources.push_back(resource);
}

storage::c_aresource_ptr_t OperationData::getResource(size_t index) const {
  return _resources.at(index);
}

const OperationData::aresource_vec_t& OperationData::all() const {
  return _resources;
}

table_list_t OperationData::getTables() const {
  return allOf<storage::AbstractTable>();
}


hash_table_list_t OperationData::getHashTables() const {
  return allOf<storage::AbstractHashTable>();
}

void OperationData::add(storage::c_atable_ptr_t input) {
  addResource(input);
}

void OperationData::addHash(storage::c_ahashtable_ptr_t input) {
  addResource(input);
}

size_t OperationData::size() const {
  return _resources.size();
}

void OperationData::setTable(storage::c_atable_ptr_t input, size_t index) {
  setNthOf(index, input);
}

void OperationData::setHash(storage::c_ahashtable_ptr_t input, size_t index) {
  setNthOf(index, input);
}

storage::c_atable_ptr_t OperationData::getTable(const size_t index) const {
  return nthOf<storage::AbstractTable>(index);
}

storage::c_ahashtable_ptr_t OperationData::getHashTable(const size_t index) const {
  return nthOf<storage::AbstractHashTable>(index);
}

size_t OperationData::numberOfTables() const {
  return sizeOf<storage::AbstractTable>();
}

size_t OperationData::numberOfHashTables() const {
  return sizeOf<storage::AbstractHashTable>();
}

bool OperationData::emptyTables() const {
  return numberOfTables() == 0;
}

bool OperationData::emptyHashTables() const {
  return numberOfHashTables() == 0;
}

void OperationData::mergeWith(OperationData &other) {
  const auto& other_res = other._resources;
  for (const auto& nextElement: other_res) {
    addResource(nextElement);
  }
}

}}
