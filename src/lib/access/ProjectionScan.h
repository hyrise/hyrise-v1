// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PROJECTIONSCAN_H_
#define SRC_LIB_ACCESS_PROJECTIONSCAN_H_

#include "PlanOperation.h"

/**
 * @brief This class implements the Projection Operator from the relational algebra.
 *
 * Allows to specify a list of fields that will be part of the results.
 */
class ProjectionScan : public _PlanOperation {
 public:

  /**
   * The constructor for the projection that takes the expected
   * field list as a parameter
   *
   * @param fields the field list
   */
  explicit ProjectionScan(field_list_t *fields): _PlanOperation() {
    
    setFields(fields);
  }

  /**
   * Default constructor
   */
  explicit ProjectionScan(): _PlanOperation() {
    
  }

  virtual ~ProjectionScan();

 protected:

  void setupPlanOperation();
  virtual void executePlanOperation();
  // virtual void teardownPlanOperation();

 public:
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);

  static std::string name() {
    return "ProjectionScan";
  }

  const std::string vname() {
    return "ProjectionScan";
  }

  static bool is_registered;

};


#endif  // SRC_LIB_ACCESS_PROJECTIONSCAN_H_

