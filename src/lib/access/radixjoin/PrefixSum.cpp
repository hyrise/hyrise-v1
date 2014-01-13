// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "PrefixSum.h"

#include "Histogram.h"
#include "access/system/QueryParser.h"

#include "storage/Table.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<PrefixSum>("PrefixSum");
}

// calculates the prefix Sum for a given table
void PrefixSum::executePlanOperation() {
   // get attribute vector of input table
  const auto &in = getInputTable();
  const size_t table_size = in->size();

  // get attribute vector of output table
  std::vector<storage::ColumnMetadata> metadata;
  metadata.push_back(in->metadataAt(0));
  auto output = std::make_shared<storage::Table>(&metadata, nullptr, table_size, true, false);
  output->resize(table_size);
  const auto &oavs = output->getAttributeVectors(0);
  auto ovector = std::dynamic_pointer_cast<storage::FixedLengthVector<value_id_t>>(oavs.at(0).attribute_vector);

  // Build ivector list to avoid lock contention while getting the vectors
  const size_t ivec_size = input.numberOfTables();
  std::vector<vec_ref_t> ivecs;
  for(size_t i=0; i < input.numberOfTables(); ++i) {
    ivecs.emplace_back(getDataVector(getInputTable(i)).first);
  }

  // calculate the prefix sum based on the index and the number of inputs
  // we need to look at to calculate the correct offset
  value_id_t sum = 0;
  for(size_t i=0; i < table_size; ++i) {
    sum += sumForIndex(ivec_size, ivecs, i);
    ovector->set(0, i, sum + sumForIndexPrev(ivec_size, ivecs, i));
  }

  addResult(output);
}

std::shared_ptr<PlanOperation> PrefixSum::parse(const Json::Value &data) {
  auto plan = std::make_shared<PrefixSum>();
  if (data.isMember("numParts")) {
    plan->_part = data["part"].asInt();
    plan->_count = data["numParts"].asInt();
  }
  return plan;
}

const std::string PrefixSum::vname() {
  return "PrefixSum";
}

void PrefixSum::splitInput() {
}

// Calculate the sum for a given index based on all the
// histograms of the input
storage::value_id_t PrefixSum::sumForIndex(const size_t ivec_size,
                                           const std::vector<vec_ref_t> &ivecs,
                                           const size_t index) const {
  storage::value_id_t sum = 0;
  for(size_t i=0, stop=ivec_size; i < stop; ++i) {
    sum += index > 0 ? ivecs.at(i)->get(0, index - 1) : 0;
  }
  return sum;
}

storage::value_id_t PrefixSum::sumForIndexPrev(const size_t ivec_size,
                                               const std::vector<vec_ref_t> &ivecs,
                                               const size_t index) const {
  storage::value_id_t sum = 0;
  for(size_t i=0, stop=ivec_size; i < stop; ++i) {
    sum += i < _part ? ivecs.at(i)->get(0, index) : 0;
  }
  return sum;
}

namespace {
  auto _2 = QueryParser::registerPlanOperation<MergePrefixSum>("MergePrefixSum");
}

void MergePrefixSum::executePlanOperation() {
  if (input.numberOfTables() == 1) {
    addResult(getInputTable());
    return;
  }

  const auto resultSize = getInputTable()->size();
  std::vector<storage::ColumnMetadata> meta {storage::ColumnMetadata::metadataFromString(types::integer_name, "count")};
  auto result = std::make_shared<storage::Table>(&meta, nullptr, resultSize, true, false);
  result->resize(resultSize);

  const auto &res_vec = getDataVector(result).first;

  std::vector<std::shared_ptr<storage::FixedLengthVector<value_id_t>>> vecs;
  for(size_t i=0, stop=input.numberOfTables(); i < stop; ++i) {
    vecs.emplace_back(getDataVector(getInputTable(i)).first);
  }

  for(size_t i=0; i < resultSize; ++i) {
    value_id_t pos = std::numeric_limits<value_id_t>::max();
    for(size_t j=0, stop=vecs.size(); j < stop; ++j) {
      auto tmp = vecs[j]->get(0,i);
      pos = tmp < pos ? tmp : pos;
    }
    res_vec->set(0, i, pos);
  }
  addResult(result);
}

std::shared_ptr<PlanOperation> MergePrefixSum::parse(const Json::Value &data) {
  return std::make_shared<MergePrefixSum>();
}

const std::string MergePrefixSum::vname() {
  return "MergePrefixSum";
}

}
}
