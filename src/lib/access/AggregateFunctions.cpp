// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "AggregateFunctions.h"

#include "json.h"

namespace hyrise {
namespace storage {
template<>
void average_aggregate_functor::operator()<std::string>() {
  throw std::runtime_error("Cannot calculate average for column of StringType");
}
}
}

aggregateFunctionMap_t getAggregateFunctionMap() {
  aggregateFunctionMap_t d;
  d["SUM"] = AggregateFunctions::SUM;
  d["COUNT"] = AggregateFunctions::COUNT;
  d["AVG"] = AggregateFunctions::AVG;
  return d;
}

AggregateFun *parseAggregateFunction(const Json::Value &data) {
  int ftype = -1;
  if (data["type"].isNumeric()) {
    ftype = data["type"].asUInt();
  } else if (data["type"].isString()) {
    ftype = getAggregateFunctionMap()[data["type"].asString()];
  }
  switch (ftype) {
    case AggregateFunctions::SUM:
      return SumAggregateFun::parse(data);
    case AggregateFunctions::COUNT:
      return CountAggregateFun::parse(data);
    case AggregateFunctions::AVG:
      return AverageAggregateFun::parse(data);
    default:
      throw std::runtime_error("Aggregation function not supported in GroupByScan");
  }
}

void AggregateFun::walk(const hyrise::storage::DCMutableTable &table) {
  if (_field_name.size() > 0) {
    _field = table.numberOfColumn(_field_name);
  }
}

AggregateFun *SumAggregateFun::parse(const Json::Value &f) {
  if (f["field"].isNumeric()) return new SumAggregateFun(f["field"].asUInt());
  else if (f["field"].isString()) return new SumAggregateFun(f["field"].asString());
  else throw std::runtime_error("Could not parse json");
};

AggregateFun *CountAggregateFun::parse(const Json::Value &f) {
  if (f["field"].isNumeric()) return new CountAggregateFun(f["field"].asUInt());
  else if (f["field"].isString()) return new CountAggregateFun(f["field"].asString());
  else throw std::runtime_error("Could not parse json");
};

AggregateFun *AverageAggregateFun::parse(const Json::Value &f) {
  if (f["field"].isNumeric()) return new AverageAggregateFun(f["field"].asUInt());
  else if (f["field"].isString()) return new AverageAggregateFun(f["field"].asString());
  else throw std::runtime_error("Could not parse json");
};
