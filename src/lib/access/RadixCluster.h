#ifndef SRC_LIB_ACCESS_RADIXCLUSTER_H_
#define SRC_LIB_ACCESS_RADIXCLUSTER_H_

#include "PlanOperation.h"
#include "Histogram.h"

#include "storage/ColumnMetadata.h"
#include "storage/hash_functor.h"
#include "storage/meta_storage.h"
#include "storage/MutableVerticalTable.h"
#include "storage/storage_types.h"

#include <cstdint>
#include <utility>

namespace hyrise { namespace access {

class CreateRadixTable : public _PlanOperation {

  static bool is_registered;

public:

  virtual ~CreateRadixTable(){}

  void executePlanOperation();

  void splitInput(){};

  const std::string vname() { return "CreateRadixTable"; }

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data) {
    return std::make_shared<CreateRadixTable>();
  }

};

/*
* This class provides a straight-forward implementation of a radix-clustering
* algorithm. The input to this operation are the required number of bits for
* the clustering, the prefix sum table from the histogram, the cluster field
* and the input table to cluster
*/
class RadixCluster : public _PlanOperation
{
public:

  RadixCluster(): _bits(0), _significantOffset(0), _start(0), _stop(0), _part(0), _count(0) {}

  virtual ~RadixCluster(){}

  void executePlanOperation();

  template<typename T>
  void executeClustering();

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);

  const std::string vname() { return "RadixCluster"; }

  inline void setBits(uint32_t b, uint32_t sig=0) {
    _bits = b;
    _significantOffset = sig;
  }

  void splitInput(){};

  inline uint32_t bits() { return _bits; }
  
  inline uint32_t significantOffset() { return _significantOffset; }

  inline void setPartInfo(int32_t p, int32_t n) { _part = p; _count = n; }

  inline void setPart(size_t p){ _part = p;}
  inline void setCount(size_t c){ _count = c;}

private:

  static bool is_registered;

  uint32_t _bits;
  uint32_t _significantOffset;

  size_t _start;
  size_t _stop;

  size_t _part;
  size_t _count;
};

template<typename T>
void RadixCluster::executeClustering() {
  
  const auto& tab = getInputTable();
  auto tableSize = tab->size();
  auto field = _field_definition[0];

  // Result Vector
  const auto& result = getInputTable(1);

  // Get the prefix sum from the input
  const auto& prefix_sum = getInputTable(2);
  const auto& data_prefix_sum = std::dynamic_pointer_cast<FixedLengthVector<value_id_t>>(getDataVector(prefix_sum).first->copy());
  
  //Prepare mask
  auto mask = ((1 << bits()) - 1) << significantOffset();
  
  // Cast the vectors to the lowest part in the hierarchy
  const auto& data_hash = getDataVector(result).first;
  const auto& data_pos = getDataVector(result, 1).first;
  
  // Calculate start stop
  _start = 0; _stop = tableSize;
  if (_count > 0) {
    _start = (tableSize / _count) * _part;
    _stop = (_count -1) == _part ? tableSize : (tableSize/_count) * (_part + 1);
  }

  // check if tab is PointerCalculator; if yes, get underlying table and actual rows and columns
    auto p = std::dynamic_pointer_cast<const PointerCalculator>(tab);
    if (p) {
      auto ipair = getDataVector(p->getActualTable());
      const auto& ivec = ipair.first;

      const auto& dict = std::dynamic_pointer_cast<OrderPreservingDictionary<T>>(tab->dictionaryAt(p->getTableColumnForColumn(field)));
      const auto& offset = p->getTableColumnForColumn(field) + ipair.second;

      auto hasher = std::hash<T>();
      for(decltype(tableSize) row = _start; row < _stop; ++row) {
        // Calculate and increment the position
        register auto hash_value  = hasher(dict->getValueForValueId(ivec->get(offset, p->getTableRowForRow(row))));//ts(tpe, fun);
        register auto offset = (hash_value & mask) >> _significantOffset;
        register auto pos_to_write = data_prefix_sum->inc(0, offset);

        // Perform the clustering
        data_hash->set(0, pos_to_write, hash_value);
        data_pos->set(0, pos_to_write, p->getTableRowForRow(row));
      }
    } else {
      auto ipair = getDataVector(tab);
      const auto& ivec = ipair.first;

      const auto& dict = std::dynamic_pointer_cast<OrderPreservingDictionary<T>>(tab->dictionaryAt(field));
      const auto& offset = field + ipair.second;

      std::hash<T> hasher;
      for(decltype(tableSize) row = _start; row < _stop; ++row) {
        // Calculate and increment the position
        register auto hash_value  = hasher(dict->getValueForValueId(ivec->get(offset, row)));//ts(tpe, fun);
        register auto offset = (hash_value & mask) >> _significantOffset;
        register auto pos_to_write = data_prefix_sum->inc(0, offset);

        // Perform the clustering
        data_hash->set(0, pos_to_write, hash_value);
        data_pos->set(0, pos_to_write, row);
      }
    }
  addResult(result);
}


/**
* Second pass on the previously radix clustered table to create the final output
* table. Input to this operation is the result of the first pass and the second
* pass of the prefix sum calculation
*/
class RadixCluster2ndPass : public _PlanOperation {

public:

  RadixCluster2ndPass() : _part(0), _count(0) {}

  virtual ~RadixCluster2ndPass(){}

  void executePlanOperation();

  const std::string vname() { return "RadixCluster2ndPass"; }

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);

  inline void setBits1(uint32_t b, uint32_t sig=0) {
    _bits1 = b; _significantOffset1 = sig;
  }

  inline void setBits2(uint32_t b, uint32_t sig=0) {
    _bits2 = b; _significantOffset2 = sig;
  }

  inline void setPart(size_t p){ _part = p;}
  inline void setCount(size_t c){ _count = c;}
  void splitInput(){};

protected:

  static bool is_registered;

  uint32_t _bits1;
  uint32_t _significantOffset1;

  uint32_t _bits2;
  uint32_t _significantOffset2;

  size_t _part;
  size_t _count;

};


}}

#endif // SRC_LIB_ACCESS_RADIXCLUSTER_H_
