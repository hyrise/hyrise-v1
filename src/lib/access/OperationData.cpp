// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "OperationData.h"

#include "storage/HashTable.h"

namespace hyrise {
namespace access {

const table_list_t &OperationData::getTables() const {
  return tables;
}


table_list_t &OperationData::getTables() {
  return tables;
}

hash_table_list_t &OperationData::getHashTables() {
  return hashTables;
}

void OperationData::add(hyrise::storage::c_atable_ptr_t input) {
  tables.push_back(input);
}

void OperationData::addHash(storage::c_ahashtable_ptr_t input) {
  hashTables.push_back(input);
}

void OperationData::setTable(storage::c_atable_ptr_t input, size_t index) {
  tables[index] = input;
}

void OperationData::setHash(storage::c_ahashtable_ptr_t input, size_t index) {
  hashTables[index] = input;
}

hyrise::storage::c_atable_ptr_t OperationData::getTable(const size_t index) const {
  return tables.at(index);
}

storage::c_ahashtable_ptr_t OperationData::getHashTable(const size_t index) const {
  return hashTables.at(index);
}

size_t OperationData::numberOfTables() const {
  return tables.size();
}

size_t OperationData::numberOfHashTables() const {
  return hashTables.size();
}

bool OperationData::emptyTables() const {
  return tables.empty();
}

bool OperationData::emptyHashTables() const {
  return hashTables.empty();
}



template <class T>
void OperationData::merge(
  const std::vector<std::shared_ptr<const T>> &ownElements,
  const std::vector<std::shared_ptr<const T>> &otherElements,
  const bool retain) {
for (const auto & nextElement: otherElements) {
    if (find(ownElements.begin(), ownElements.end(), nextElement)
        ==  ownElements.end()) {
      addHash(nextElement);
    }
  }
}

void OperationData::mergeWith(OperationData &other, const bool retain) {

for (const auto& nextElement: other.getTables()) {

//    if (find(getTables().begin(), getTables().end(), nextElement)
//        ==  getTables().end())

      add(nextElement);
  }

  merge<AbstractHashTable>(getHashTables(), other.getHashTables(), retain);

}

}}
