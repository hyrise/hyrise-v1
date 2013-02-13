#ifndef SRC_LIB_ACCESS_RADIXCLUSTER_H_
#define SRC_LIB_ACCESS_RADIXCLUSTER_H_

#include "PlanOperation.h"

#include <cstdint>

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
