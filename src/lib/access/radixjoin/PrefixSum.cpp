// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "PrefixSum.h"

#include "Histogram.h"
#include "access/system/QueryParser.h"

#include "storage/Table.h"

#include "helper/checked_cast.h"

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<PrefixSum>("PrefixSum");
}

// calculates the prefix Sum for a given table
void PrefixSum::executePlanOperation() {
  // get attribute vector of input table
  const auto& in = getInputTable();
  const size_t table_size = in->size();

  // get attribute vector of output table
  std::vector<storage::ColumnMetadata> metadata;
  metadata.push_back(in->metadataAt(0));
  auto output = std::make_shared<storage::Table>(&metadata, nullptr, table_size, true, false);
  output->resize(table_size);
  const auto& oavs = output->getAttributeVectors(0);
  auto ovector = checked_pointer_cast<vec_t>(oavs.at(0).attribute_vector);

  // Build ivector list to avoid lock contention while getting the vectors
  const size_t ivec_size = input.numberOfTables();
  std::vector<vec_ref_t> ivecs;
  for (size_t i = 0; i < ivec_size; ++i) {
    ivecs.emplace_back(checked_pointer_cast<vec_t>(getFixedDataVector(getInputTable(i)).first));
  }

  // calculate the prefix sum based on the index and the number of inputs
  // we need to look at to calculate the correct offset
  std::vector<value_id_t> vec_all;
  std::vector<value_id_t> vec_prev;
  vec_all.resize(table_size);
  vec_prev.resize(table_size);

  value_id_t v = 0;
  for (size_t i = 0; i < ivec_size; ++i) {
    for (size_t j = 0; j < table_size; ++j) {
      v = ivecs[i]->get(0, j);
      if (i < _part)
        vec_prev[j] += v;
      else
        vec_all[j] += v;
    }
  }

  // std::pair<storage::value_id_t,storage::value_id_t> sum_p(0,0);
  // value_id_t prev_sum = 0;
  value_id_t sum_n = 0;
  for (size_t i = 0; i < table_size; ++i) {
    vec_all[i] += vec_prev[i];
    sum_n += i > 0 ? vec_all[i - 1] : 0;
    ovector->set(0, i, sum_n + vec_prev[i]);
  }

  addResult(output);
}

std::shared_ptr<PlanOperation> PrefixSum::parse(const Json::Value& data) {
  auto plan = std::make_shared<PrefixSum>();
  if (data.isMember("numParts")) {
    plan->_part = data["part"].asInt();
    plan->_count = data["numParts"].asInt();
  }
  return plan;
}

const std::string PrefixSum::vname() { return "PrefixSum"; }

void PrefixSum::splitInput() {}

namespace {
auto _2 = QueryParser::registerPlanOperation<MergePrefixSum>("MergePrefixSum");
}

void MergePrefixSum::executePlanOperation() {
  if (input.numberOfTables() == 1) {
    addResult(getInputTable());
    return;
  }

  const auto resultSize = getInputTable()->size();
  std::vector<storage::ColumnMetadata> meta{storage::ColumnMetadata::metadataFromString(types::integer_name, "count")};
  auto result = std::make_shared<storage::Table>(&meta, nullptr, resultSize, true, false);
  result->resize(resultSize);

  const auto& res_vec = getFixedDataVector(result).first;

  std::vector<std::shared_ptr<storage::AbstractFixedLengthVector<value_id_t>>> vecs;
  for (size_t i = 0, stop = input.numberOfTables(); i < stop; ++i) {
    vecs.emplace_back(getFixedDataVector(getInputTable(i)).first);
  }

  for (size_t i = 0; i < resultSize; ++i) {
    value_id_t pos = std::numeric_limits<value_id_t>::max();
    for (size_t j = 0, stop = vecs.size(); j < stop; ++j) {
      auto tmp = vecs[j]->get(0, i);
      pos = tmp < pos ? tmp : pos;
    }
    res_vec->set(0, i, pos);
  }
  addResult(result);
}

std::shared_ptr<PlanOperation> MergePrefixSum::parse(const Json::Value& data) {
  return std::make_shared<MergePrefixSum>();
}

const std::string MergePrefixSum::vname() { return "MergePrefixSum"; }
}
}
