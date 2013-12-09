// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/MaterializingScan.h"

#include <random>
#include <set>

#include "access/system/BasicParser.h"
#include "access/system/QueryParser.h"

#include "storage/Table.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<MaterializingScan>("MaterializingScan");
}

MaterializingScan::MaterializingScan(const bool use_memcpy) :
                                     _use_memcpy(use_memcpy),
                                     _copy_values(false),
                                     _num_samples(0) {
}

MaterializingScan::~MaterializingScan() {
}

void MaterializingScan::setupPlanOperation() {
  // Call super impl
  PlanOperation::setupPlanOperation();

  // build sample
  if (_num_samples > 0) {
    std::mt19937 gen;
    gen.seed(static_cast<unsigned int>(std::time(0)));
    std::uniform_int_distribution<unsigned> dist(0, input.getTable(0)->size() - 1);

    std::set<unsigned> base;
    while (base.size() < _num_samples) {
      unsigned val = dist(gen);
      std::pair<std::set<unsigned>::iterator, bool> tmp = base.insert(val);
      if (tmp.second)
        _samples.push_back(val);
    }
  }
}

void MaterializingScan::executePlanOperation() {
  const auto& in = input.getTable(0);
  auto result = std::dynamic_pointer_cast<storage::Table>(in->copy_structure(nullptr, true, in->size(), false));

  if (_num_samples == 0) {
    result->resize(in->size());
    for (size_t row = 0; row < in->size(); row++) {
      result->copyRowFrom(in, row, row, false, false /*_use_memcpy*/);
    }
  } else {
    result->resize(_num_samples);
    for (size_t row = 0; row < _samples.size(); row++) {
      result->copyRowFrom(input.getTable(0), _samples[row], row, _copy_values, false /*_use_memcpy*/);
    }
  }

  addResult(result);
}

std::shared_ptr<PlanOperation> MaterializingScan::parse(const Json::Value &v) {
  std::shared_ptr<MaterializingScan> pop = std::dynamic_pointer_cast<MaterializingScan>(BasicParser<MaterializingScan>::parse(v));
  if (v.isMember("samples"))
    pop->setSamples(v["samples"].asUInt());

  if (v.isMember("memcpy"))
    pop->_use_memcpy = v["memcpy"].asBool();
  else
    pop->_use_memcpy = false;

  if (v.isMember("copyValues"))
    pop->setCopyValues(v["copyValues"].asBool());

  return pop;
}

const std::string MaterializingScan::vname() {
  return "MaterializingScan";
}

void MaterializingScan::setSamples(const unsigned s) {
  _num_samples = s;
}

void MaterializingScan::setCopyValues(const bool v) {
  _copy_values = v;
}

}
}
