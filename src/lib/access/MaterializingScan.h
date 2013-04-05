// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_MATERIALIZINGSCAN_H_
#define SRC_LIB_ACCESS_MATERIALIZINGSCAN_H_

#include "access/PlanOperation.h"

namespace hyrise {
namespace access {

class MaterializingScan : public _PlanOperation {
public:
  explicit MaterializingScan(const bool use_memcpy = true);
  virtual ~MaterializingScan();

  void setupPlanOperation();
  void executePlanOperation();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &v);
  const std::string vname();
  void setSamples(const unsigned s);
  void setCopyValues(const bool v);

private:
  bool _use_memcpy;
  bool _copy_values;
  unsigned _num_samples;
  std::vector<unsigned> _samples;
};

}
}

#endif  // SRC_LIB_ACCESS_MATERIALIZINGSCAN_H_
