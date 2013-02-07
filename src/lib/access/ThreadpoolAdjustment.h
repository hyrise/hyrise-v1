// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_THREADPOOLADJUSTMENT_H_
#define SRC_LIB_ACCESS_THREADPOOLADJUSTMENT_H_

#include <access/PlanOperation.h>

/*  This operation offers interfaces to adjust the configuration of the
    threadpool. Use this as long as there is no dedicated scheduling unit.  */
class ThreadpoolAdjustment : public _PlanOperation {
  size_t size;
 public:
  static bool is_registered;

  ThreadpoolAdjustment() :
      size(1) {

  }

  virtual ~ThreadpoolAdjustment() {

  }

  static std::string name() {
    return "ThreadpoolAdjustment";
  }
  const std::string vname() {
    return "ThreadpoolAdjustment";
  }

  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);

  void executePlanOperation();

  //  Set the maximum number of parallel executable operation tasks.
  void setThreadpoolSize(const size_t newSize);
};

#endif  // SRC_LIB_ACCESS_THREADPOOLADJUSTMENT_H_

