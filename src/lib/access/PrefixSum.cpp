// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "QueryParser.h"
#include "PrefixSum.h"
#include "Histogram.h"
#include "storage/Table.h"

#include <limits>

namespace hyrise {
namespace access {

bool PrefixSum::is_registered = QueryParser::registerPlanOperation<PrefixSum>("PrefixSum");
bool MergePrefixSum::is_registered = QueryParser::registerPlanOperation<MergePrefixSum>("MergePrefixSum");

std::shared_ptr<_PlanOperation> PrefixSum::parse(Json::Value &data) {
  auto plan = std::make_shared<PrefixSum>();
  if (data.isMember("numParts")) {
    plan->_part = data["part"].asInt();
    plan->_count = data["numParts"].asInt();
  }
  return plan;
}

// Calculate the sum for a given index based on all the 
// histograms of the input
value_id_t PrefixSum::sumForIndex(const size_t ivec_size, std::vector<vec_ref_t>& ivecs, size_t index) {
  value_id_t sum = 0;
  for(size_t i=0, stop=ivec_size; i < stop; ++i) {
    sum += index > 0 ? ivecs.at(i)->get(0, index - 1) : 0;
    //    sum += i < _part ? vect->get(0, index) : 0;
  }
  return sum;
}

value_id_t PrefixSum::sumForIndexPrev(const size_t ivec_size, std::vector<vec_ref_t>& ivecs, size_t index) {
  value_id_t sum = 0;
  for(size_t i=0, stop=ivec_size; i < stop; ++i) {
    sum += i < _part ? ivecs.at(i)->get(0, index) : 0;
  }
  return sum;
}



// calculates the prefix Sum for a given table 
void PrefixSum::executePlanOperation() {
   // get attribute vector of input table
  const auto& in = getInputTable();
  size_t table_size = in->size();

  // get attribute vector of output table
  std::vector<const ColumnMetadata *> metadata;
  metadata.push_back(in->metadataAt(0));
  auto output = std::make_shared<Table<>>(&metadata, nullptr, table_size, true, 0, 0, false);
  output->resize(table_size);
  const auto& oavs = output->getAttributeVectors(0);
  auto ovector = std::dynamic_pointer_cast<FixedLengthVector<value_id_t>>(oavs.at(0).attribute_vector);

  // Build ivector list to avoid lock contention while getting the vectors
  size_t ivec_size = input.numberOfTables();
  std::vector<vec_ref_t> ivecs;
  for(size_t i=0; i < input.numberOfTables(); ++i) {
    //const auto& tmp = getDataVector(getInputTable(i)).first;
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

void MergePrefixSum::executePlanOperation() {

  if (input.numberOfTables() == 1) {
    addResult(getInputTable());
    return;
  }

  const auto resultSize = getInputTable()->size();
  std::vector<const ColumnMetadata*> meta {ColumnMetadata::metadataFromString(hyrise::types::integer_t, "count")};
  auto result = std::make_shared<Table<>>(&meta, nullptr, resultSize, true, 0, 0, false);
  result->resize(resultSize);

  const auto& res_vec = getDataVector(result).first;

  std::vector<std::shared_ptr<FixedLengthVector<value_id_t>>> vecs;
  for(size_t i=0, stop=input.numberOfTables(); i < stop; ++i) {
    vecs.emplace_back(getDataVector(getInputTable(i)).first);
  }

  for(size_t i=0; i < resultSize; ++i) {
    value_id_t pos = std::numeric_limits<value_id_t>::max();
    for(size_t j=0, stop=vecs.size(); j < stop; ++j) {
      register auto tmp = vecs[j]->get(0,i);
      pos = tmp < pos ? tmp : pos;
    }
    res_vec->set(0, i, pos);
  }
  addResult(result);
}

}
}
