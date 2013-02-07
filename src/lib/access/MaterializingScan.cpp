// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <boost/random.hpp>

#include <set>

#include <access/MaterializingScan.h>
#include <storage/Table.h>

#include "QueryParser.h"

bool MaterializingScan::is_registered = QueryParser::registerPlanOperation<MaterializingScan>();


void MaterializingScan::setupPlanOperation() {
  // Call super impl
  _PlanOperation::setupPlanOperation();

  // build sample
  if (_num_samples > 0) {
    boost::mt19937 gen;
    gen.seed(static_cast<unsigned int>(std::time(0)));

    boost::uniform_int<> dist(0, input.getTable(0)->size() - 1);
    boost::variate_generator<boost::mt19937 &, boost::uniform_int<> > die(gen, dist);

    std::set<unsigned> base;
    while (base.size() < _num_samples) {
      unsigned val = die();
      std::pair<std::set<unsigned>::iterator, bool> tmp = base.insert(val);
      if (tmp.second)
        _samples.push_back(val);
    }
  }
}


void MaterializingScan::executePlanOperation() {

  const auto& in = input.getTable(0);
  auto result = std::dynamic_pointer_cast<Table<>>(in->copy_structure(nullptr, true, in->size(), false));


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

