#include "RadixCluster.h"

#include "access/BasicParser.h"
#include "access/Histogram.h"
#include "access/QueryParser.h"

#include "helper/types.h"

#include "storage/ColumnMetadata.h"
#include "storage/hash_functor.h"
#include "storage/meta_storage.h"
#include "storage/MutableVerticalTable.h"
#include "storage/storage_types.h"

//#include <gperftools/profiler.h>

namespace hyrise { namespace access {

bool RadixCluster::is_registered = QueryParser::registerPlanOperation<RadixCluster>("RadixCluster");
bool RadixCluster2ndPass::is_registered = QueryParser::registerPlanOperation<RadixCluster2ndPass>("RadixCluster2ndPass");
bool CreateRadixTable::is_registered = QueryParser::registerPlanOperation<CreateRadixTable>("CreateRadixTable");

void CreateRadixTable::executePlanOperation() {
  auto tab = getInputTable();
  auto tableSize = getInputTable()->size();

  // Prepare output
  std::vector<const ColumnMetadata*> meta1 {ColumnMetadata::metadataFromString(hyrise::types::integer_t, "hash")};
  std::vector<const ColumnMetadata*> meta2 {ColumnMetadata::metadataFromString(hyrise::types::integer_t, "pos")};

  // Create the result tables
  auto hashes = std::make_shared<Table<>>(&meta1, nullptr, tableSize, true, 0, 0, false);
  hashes->resize(tableSize);

  auto positions = std::make_shared<Table<>>(&meta2, nullptr, tableSize, true, 0, 0, false);
  positions->resize(tableSize);

  std::vector<hyrise::storage::atable_ptr_t> tmp {hashes, positions};
  auto result = std::make_shared<MutableVerticalTable>(tmp);
  addResult(result);
}

void RadixCluster::executePlanOperation() {
  
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

  // Prepare hashing
  // radix_hash_value<size_t> fun(tab, field);
  // hyrise::storage::type_switch<hyrise_basic_types> ts;
  // auto tpe = tab->typeOfColumn(field);

  auto ipair = getDataVector(tab);
  const auto& ivec = ipair.first;

  const auto& dict = std::dynamic_pointer_cast<OrderPreservingDictionary<hyrise_int_t>>(tab->dictionaryAt(field));
  const auto& offset = field + ipair.second;

  
  // Cast the vectors to the lowest part in the hierarchy
  const auto& data_hash = getDataVector(result).first;
  const auto& data_pos = getDataVector(result, 1).first;
  
  // Calculate start stop
  _start = 0; _stop = tableSize;
  if (_count > 0) {
    _start = (tableSize / _count) * _part;
    _stop = (_count -1) == _part ? tableSize : (tableSize/_count) * (_part + 1);
  }

  std::hash<value_id_t> hasher;
  //std::cout << _operatorId + " " + std::to_string(_start) + " " + std::to_string(_stop) + "\n";
  for(decltype(tableSize) row = _start; row < _stop; ++row) {
    // Hash the value
    //fun.setRow(row);

    // Calculate and increment the position
    register auto hash_value  = hasher(dict->getValueForValueId(ivec->get(offset, row)));//ts(tpe, fun);
    register auto offset = (hash_value & mask) >> _significantOffset;
    register auto pos_to_write = data_prefix_sum->inc(0, offset);

    // Perform the clustering
    data_hash->set(0, pos_to_write, hash_value);
    data_pos->set(0, pos_to_write, row);

    //std::cout << _operatorId + " " + std::to_string(hash_value) + " " + std::to_string(row) + " " + std::to_string(pos_to_write) + "\n";
  }
  addResult(result);
}

std::shared_ptr<_PlanOperation> RadixCluster::parse(Json::Value &data) {
  auto hst = BasicParser<RadixCluster>::parse(data);
  hst->setBits(data["bits"].asUInt(), data["sig"].asUInt());
  if (data.isMember("numParts")) {
    hst->_part = data["part"].asInt();
    hst->_count = data["numParts"].asInt();
  } 
  return hst;
}

std::shared_ptr<_PlanOperation> RadixCluster2ndPass::parse(Json::Value &data) {
  auto hst = BasicParser<RadixCluster2ndPass>::parse(data);
  hst->setBits1(data["bits"].asUInt(), data["sig"].asUInt());
  hst->setBits2(data["bits2"].asUInt(), data["sig2"].asUInt());

  if (data.isMember("numParts")) {
    hst->setPart(data["part"].asUInt());
    hst->setCount(data["numParts"].asUInt());
  } else {
    hst->setPart(0);
    hst->setCount(0);
  }
  return hst;
}

void RadixCluster2ndPass::executePlanOperation() {

  //ProfilerStart("RadixCluster.prof");

  assert(_bits1 != 0);
  assert(_bits2 != 0);

  const auto& tab = getInputTable();
  auto tableSize = getInputTable()->size();

  auto result = getInputTable(1);

  // Get the prefix sum from the input
  const auto& in_data = getDataVector(getInputTable(2)).first;

  auto prefix = std::dynamic_pointer_cast<FixedLengthVector<value_id_t>>(in_data->copy());
  
  // Cast the vectors to the lowest part in the hierarchy
  auto data_hash = getDataVector(result).first;
  auto data_pos = getDataVector(result, 1).first;

  // Get the check data
  const auto& rx_hashes = getDataVector(tab).first;
  const auto& rx_pos = getDataVector(tab, 1).first;
  
  auto mask1 = ((1 << _bits1) - 1) << _significantOffset1;
  auto mask2 = ((1 << _bits2) - 1) << _significantOffset2;

  size_t _start = 0, _stop = tableSize;
  if (_count > 0) {
    _start = (tableSize / _count) * _part;
    _stop = (_count -1) == _part ? tableSize : (tableSize/_count) * (_part + 1);
  }

  // Iterate over the first pass radix clustered table and write the 
  // newly clustered results
  for(size_t row=_start; row < _stop; ++row) {
    const auto hash_value = rx_hashes->get(0, row);
    const auto part1 = (hash_value & mask1) >> _significantOffset1;
    const auto part2 = (hash_value & mask2) >> _significantOffset2;
    const auto lookup = part1 * (1 << _bits2) + part2;
    const auto pos_to_write = prefix->inc(0, lookup);
    data_hash->set(0, pos_to_write, hash_value);
    data_pos->set(0, pos_to_write, rx_pos->get(0, row));
  }
  //ProfilerStop();

  addResult(result);
}

}}
