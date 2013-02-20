#include "RadixCluster.h"

#include "access/BasicParser.h"

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
  switch(getInputTable()->typeOfColumn(_field_definition[0])) {
    case IntegerType:
      executeClustering<hyrise_int_t>();
      break;
    case FloatType:
      executeClustering<hyrise_float_t>();
      break;
    case StringType:
      executeClustering<hyrise_string_t>();
      break;
  }
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
