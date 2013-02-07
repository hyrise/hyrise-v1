#include "OperationData.h"

#include "storage/HashTable.h"

OperationData::~OperationData() {
}

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

void OperationData::addHash(std::shared_ptr<AbstractHashTable> input) {
  hashTables.push_back(input);
}

hyrise::storage::c_atable_ptr_t OperationData::getTable(const size_t index) const {
  return tables.at(index);
}

std::shared_ptr<AbstractHashTable> OperationData::getHashTable(const size_t index) const {
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
  const std::vector<std::shared_ptr<T>> &ownElements,
  const std::vector<std::shared_ptr<T>> &otherElements,
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
    if (find(getTables().begin(), getTables().end(), nextElement)
        ==  getTables().end())
      add(nextElement);
  }

  merge<AbstractHashTable>(getHashTables(), other.getHashTables(), retain);

}

