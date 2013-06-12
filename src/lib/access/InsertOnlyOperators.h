// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_INSERTONLYOPERATORS_H_
#define SRC_LIB_ACCESS_INSERTONYLOPERATORS_H_

#include "helper/types.h"
#include "access/system/PlanOperation.h"

namespace hyrise {
namespace insertonly {

/// Loads an insertonly table
class LoadOp : public _PlanOperation {
 public:
  explicit LoadOp(const std::string& filename);
  const std::string vname();
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  virtual void executePlanOperation();
private:
  const std::string _filename;
};

/// Insert rows into insert only table
class InsertOp : public _PlanOperation {
 public:
  const std::string vname();

  /// Allowed parameters
  /// Json parameters: none
  /// Query graph inputs:
  /// 1. store
  /// 2. rows
  /// Query graph output:
  /// - store with new rows
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  virtual void executePlanOperation();
};

/// Updates rows in store
class UpdateOp : public _PlanOperation {
 public:
  const std::string vname();

  /// Allowed parameters
  /// Json parameters: none
  /// Query graph inputs:
  /// 1. store
  /// 2. updated rows
  /// 3. positions to update in main (as pointer calculator)
  /// 4. positions to update in delta (as pointer calculator)
  /// Query graph output:
  /// - store with new rows
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  virtual void executePlanOperation();
};

/// Invalidates rows in store
class DeleteOp : public _PlanOperation {
 public:
  const std::string vname();

  /// Allowed parameters
  /// Json parameters: none
  /// Query graph inputs:
  /// 1. store
  /// 2. positions to invalidate in main
  /// 3. positions to invalidate in delta
  /// Query graph output:
  /// - store with invalidated rows
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  virtual void executePlanOperation();
};

/// Extracts valid positions from delta
class ValidPositionsRawOp : public _PlanOperation {
 public:
  const std::string vname();

  /// Allowed parameters
  /// Json parameters: none
  /// Query graph inputs:
  /// 1. store
  /// Query graph output:
  /// 1. Pointercalculator with valid rows as positions
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  virtual void executePlanOperation();
};

/// Extracts valid positions from main
class ValidPositionsMainOp : public _PlanOperation {
 public:
  const std::string vname();

  /// Query graph inputs:
  /// 1. store
  /// Query graph output:
  /// 1. Pointercalculator with valid rows as positions
  static std::shared_ptr<_PlanOperation> parse(Json::Value &data);
  virtual void executePlanOperation();
};

/// Operation to extract delta RawTable from SimpleStore
class ExtractDelta: public _PlanOperation {
 public:

  /// Query graph inputs:
  /// 1. store
  /// Query graph output:
  /// 1. rawatable<>
  static std::shared_ptr<_PlanOperation> parse(Json::Value& data);
  void executePlanOperation();
  virtual const std::string vname();
};


}}

#endif
