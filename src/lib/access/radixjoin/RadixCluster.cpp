#include "RadixCluster.h"

#include "access/system/BasicParser.h"
#include "access/system/QueryParser.h"

#include "storage/ColumnMetadata.h"
#include "storage/MutableVerticalTable.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<CreateRadixTable>("CreateRadixTable");
}

void CreateRadixTable::executePlanOperation() {
  auto tab = getInputTable();
  auto tableSize = getInputTable()->size();

  // Prepare output
  std::vector<storage::ColumnMetadata> meta1 {storage::ColumnMetadata::metadataFromString(types::integer_name, "hash")};
  std::vector<storage::ColumnMetadata> meta2 {storage::ColumnMetadata::metadataFromString(types::integer_name, "pos")};

  // Create the result tables
  auto hashes = std::make_shared<storage::Table>(&meta1, nullptr, tableSize, true, false);
  hashes->resize(tableSize);

  auto positions = std::make_shared<storage::Table>(&meta2, nullptr, tableSize, true, false);
  positions->resize(tableSize);

  std::vector<storage::atable_ptr_t> tmp {hashes, positions};
  auto result = std::make_shared<storage::MutableVerticalTable>(tmp);

  addResult(result);
}

std::shared_ptr<PlanOperation> CreateRadixTable::parse(const Json::Value &data) {
  return std::make_shared<CreateRadixTable>();
}

const std::string CreateRadixTable::vname() {
  return "CreateRadixTable";
}

namespace {
  auto _2 = QueryParser::registerPlanOperation<RadixCluster>("RadixCluster");
}

RadixCluster::RadixCluster(): _bits(0),
                              _significantOffset(0),
                              _start(0),
                              _stop(0),
                              _part(0),
                              _count(0) {
}

void RadixCluster::executePlanOperation() {
  switch(getInputTable()->typeOfColumn(_field_definition[0])) {
  case IntegerType:
  case IntegerTypeDelta:
  case IntegerTypeDeltaConcurrent:
    executeClustering<storage::hyrise_int_t>();
    break;
  case IntegerNoDictType:
    executeClustering<storage::hyrise_int32_t>();
    break;
  case FloatType:
  case FloatTypeDelta:
  case FloatTypeDeltaConcurrent:
  case FloatNoDictType:
    executeClustering<storage::hyrise_float_t>();
    break;
  case StringType:
  case StringTypeDelta:
  case StringTypeDeltaConcurrent:
    executeClustering<storage::hyrise_string_t>();
    break;
  }
}

std::shared_ptr<PlanOperation> RadixCluster::parse(const Json::Value &data) {
  auto hst = BasicParser<RadixCluster>::parse(data);
  hst->setBits(data["bits"].asUInt(), data["sig"].asUInt());
  if (data.isMember("numParts")) {
    hst->_part = data["part"].asInt();
    hst->_count = data["numParts"].asInt();
  }
  return hst;
}

const std::string RadixCluster::vname() {
  return "RadixCluster";
}

void RadixCluster::setPart(const size_t p) {
  _part = p;
}

void RadixCluster::setCount(const size_t c) {
  _count = c;
}

void RadixCluster::setPartInfo(const int32_t p,
                               const int32_t n) {
  _part = p;
  _count = n;
}

void RadixCluster::setBits(const uint32_t b,
                           const uint32_t sig) {
  _bits = b;
  _significantOffset = sig;
}

uint32_t RadixCluster::bits() const {
  return _bits;
}

uint32_t RadixCluster::significantOffset() const {
  return _significantOffset;
}

namespace {
  auto _3 = QueryParser::registerPlanOperation<RadixCluster2ndPass>("RadixCluster2ndPass");
}

RadixCluster2ndPass::RadixCluster2ndPass() : _part(0),
                                             _count(0) {
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

  auto prefix = std::dynamic_pointer_cast<storage::FixedLengthVector<value_id_t>>(in_data->copy());

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

std::shared_ptr<PlanOperation> RadixCluster2ndPass::parse(const Json::Value &data) {
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

const std::string RadixCluster2ndPass::vname() {
  return "RadixCluster2ndPass";
}

void RadixCluster2ndPass::setBits1(const uint32_t b,
                                   const uint32_t sig) {
  _bits1 = b;
  _significantOffset1 = sig;
}

void RadixCluster2ndPass::setBits2(const uint32_t b,
                                   const uint32_t sig) {
  _bits2 = b;
  _significantOffset2 = sig;
}

void RadixCluster2ndPass::setPart(const size_t p) {
  _part = p;
}

void RadixCluster2ndPass::setCount(const size_t c) {
  _count = c;
}

}
}
