// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "Histogram.h"

#include "access/system/BasicParser.h"
#include "access/system/QueryParser.h"

#include "storage/ColumnMetadata.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<Histogram>("Histogram");
}

Histogram::Histogram() : _bits(0),
                         _significantOffset(0),
                         _bits2(0),
                         _significantOffset2(0),
                         _part(0),
                         _count(0) {
}

// The histogram basically works by iterating over the table and the requested
// field, hashing the value and incresing the count for this entry
void Histogram::executePlanOperation() {
  switch(getInputTable()->typeOfColumn(_field_definition[0])) {
  case IntegerType:
  case IntegerTypeDelta:
  case IntegerTypeDeltaConcurrent:
    return executeHistogram<storage::hyrise_int_t>();
  case IntegerNoDictType:
    return executeHistogram<storage::hyrise_int32_t>();
  case FloatType:
  case FloatTypeDelta:
  case FloatTypeDeltaConcurrent:
  case FloatNoDictType:
    return executeHistogram<storage::hyrise_float_t>();
  case StringType:
  case StringTypeDelta:
  case StringTypeDeltaConcurrent:
    return executeHistogram<storage::hyrise_string_t>();
  }
}

std::shared_ptr<PlanOperation> Histogram::parse(const Json::Value &data) {
  auto hst = BasicParser<Histogram>::parse(data);
  hst->setBits(data["bits"].asUInt(), data["sig"].asUInt());
  if (data.isMember("numParts")) {
    hst->_part = data["part"].asInt();
    hst->_count = data["numParts"].asInt();
  }
  return hst;
}

const std::string Histogram::vname() {
  return "Histogram";
}

void Histogram::setPart(const size_t p) {
  _part = p;
}

void Histogram::setCount(const size_t c) {
  _count = c;
}

void Histogram::setBits(const uint32_t b,
                        const uint32_t sig) {
  _bits = b;
  _significantOffset = sig;
}

void Histogram::setBits2(const uint32_t b,
                         const uint32_t sig) {
  _bits2 = b;
  _significantOffset2 = sig;
}

uint32_t Histogram::bits() const {
  return _bits;
}

uint32_t Histogram::significantOffset() const {
  return _significantOffset;
}

std::shared_ptr<storage::Table> Histogram::createOutputTable(const size_t size) const {
  std::vector<storage::ColumnMetadata> meta {storage::ColumnMetadata::metadataFromString(types::integer_name, "count")};
  auto result = std::make_shared<storage::Table>(&meta, nullptr, size, true, false);
  result->resize(size);
  return result;
}

namespace {
  auto _2 = QueryParser::registerPlanOperation<Histogram2ndPass>("Histogram2ndPass");
}

void Histogram2ndPass::executePlanOperation() {
  const auto &tab = getInputTable();
  const auto tableSize = getInputTable()->size();
  const auto field = 0;//_field_definition[0];

  //Prepare mask
  auto mask = ((1 << bits()) - 1) << significantOffset();
  auto mask2 = (( 1 << _bits2 ) - 1) << _significantOffset2;

  // Prepare Output Table
  auto result = createOutputTable((1<<_bits2) * (1 << _bits));
  auto o_data = getDataVector(result).first;

  // Get input vector
  auto i_data = getDataVector(tab).first;

  // Iterate and hash based on the part description
  size_t start=0, stop=tableSize;
  if (_count > 0) {
    start = (tableSize / _count) * _part;
    stop = (_count -1) == _part ? tableSize : (tableSize/_count) * (_part + 1);
  }
  for(size_t row = start; row < stop; ++row) {
    auto hash_value = i_data->get(field, row);
    auto offset = (hash_value & mask) * (1<<_bits2) + ((hash_value & mask2) >> _significantOffset2);
    o_data->inc(0, offset);
  }

  addResult(result);
}

std::shared_ptr<PlanOperation> Histogram2ndPass::parse(const Json::Value &data) {
  auto hst = BasicParser<Histogram2ndPass>::parse(data);
  hst->setBits(data["bits"].asUInt(), data["sig"].asUInt());
  hst->setBits2(data["bits2"].asUInt(), data["sig2"].asUInt());
  if (data.isMember("numParts")) {
    hst->setPart(data["part"].asInt());
    hst->setCount(data["numParts"].asInt());
  }
  return hst;
}

const std::string Histogram2ndPass::vname() {
  return "Histogram2ndPass";
}

}
}
