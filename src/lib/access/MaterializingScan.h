// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_MATERIALIZINGSCAN_H_
#define SRC_LIB_ACCESS_MATERIALIZINGSCAN_H_

#include <vector>
#include <access/PlanOperation.h>
#include "BasicParser.h"

class MaterializingScan : public _PlanOperation {
 private:

  bool _use_memcpy;
  bool _copy_values;
  unsigned _num_samples;

  std::vector<unsigned> _samples;

 public:

  explicit MaterializingScan(bool use_memcpy = true) : _use_memcpy(use_memcpy), _copy_values(false), _num_samples(0) {
    
  }

  virtual ~MaterializingScan() { }

  static std::shared_ptr<_PlanOperation> parse(Json::Value &v) {
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

  static std::string name() {
    return "MaterializingScan";
  }
  static bool is_registered;

  const std::string vname() {
    return "MaterializingScan";
  }

  void setSamples(unsigned s) {
    _num_samples = s;
  }

  void setCopyValues(bool v) {
    _copy_values = v;
  }

  virtual void setupPlanOperation();

  virtual void executePlanOperation();

};

#endif  // SRC_LIB_ACCESS_MATERIALIZINGSCAN_H_

