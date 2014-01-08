// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_PLANOPERATION_H_
#define SRC_LIB_ACCESS_PLANOPERATION_H_

#include "access/system/OutputTask.h"
#include "access/system/OperationData.h"
#include "access/system/QueryParser.h"
#include "io/TXContext.h"

#include "storage/storage_types.h"
#include "helper/types.h"
#include "storage/AbstractTable.h"

#include "json.h"


namespace hyrise {
namespace access {

class ResponseTask;

/**
 * This is the default interface for a plan operation. Our basic assumption is
 * that a plan operation has multiple input data structures and exactly one
 * output data structure.
 */
class PlanOperation : public OutputTask {
 protected:
  void addResult(storage::c_aresource_ptr_t result);

  void computeDeferredIndexes();

  /*!
   *  Fetches output data of dependencies as input data.
   */
  virtual void refreshInput();

  virtual void setupPlanOperation();
  virtual void executePlanOperation() = 0;
  virtual void teardownPlanOperation() {}

  /* Returns true when none of the dependencies have OpFail state */
  bool allDependenciesSuccessful();

  /* Returns all errors of dependencies as one concatenated std::string */
  std::string getDependencyErrorMessages();


  /* 
   * The model used is based on a/x + b as an equation, whereas x
   * is the number of instances used. The result is the mean task execution time
   * for this operator. a and b are parameters based on the input table size.
   * b also denotes the minimal possible mean task execution time.
   * It thus sets the minimal achievable mts.
   * for determineDynamicCount 
   */
  virtual size_t getTotalTableSize();
  /* determine the b parameter also known as minimal achievable mts */
  virtual double calcMinMts(double totalTblSizeIn100k);
  /* determine the a parameter of the model. */
  virtual double calcA(double totalTblSizeIn100k);
  /*
   * The standard implementation of the calc* method assume
   * a straight line model with a*x + b.
   * You can either override the following parameters in your operator
   * or you can supply your calc* methods.
   */
  virtual double min_mts_a() { return 0; }
  virtual double min_mts_b() { return 0; }
  virtual double a_a() { return 0; }
  virtual double a_b() { return 0; }

 public:
  virtual ~PlanOperation();

  virtual size_t determineDynamicCount(size_t maxTaskRunTime);

  void setLimit(uint64_t l);
  void setProducesPositions(bool p);
  
  void setTXContext(tx::TXContext ctx);

  void addInput(storage::c_aresource_ptr_t t);

  const storage::c_atable_ptr_t getInputTable(size_t index = 0) const;
  const storage::c_atable_ptr_t getResultTable(size_t index = 0) const;
  storage::c_ahashtable_ptr_t getInputHashTable(size_t index = 0) const;
  storage::c_ahashtable_ptr_t getResultHashTable(size_t index = 0) const;

  void addField(field_t field);
  void addField(const Json::Value &field);
  void addNamedField(const field_name_t& field);

  void setPlanId(std::string i);
  void setOperatorId(std::string i);
  const std::string& planOperationName() const;
  void setPlanOperationName(const std::string& name);

  virtual void operator()() noexcept;
  virtual const std::string vname();
  const PlanOperation *execute();

  void setErrorMessage(const std::string& message);
  void setResponseTask(const std::shared_ptr<ResponseTask>& responseTask);
  std::shared_ptr<ResponseTask> getResponseTask() const;
 protected:
  /// Containers to store and handle input/output or rather result data.
  OperationData input;
  OperationData output;

  /// Limits the number of rows read
  uint64_t _limit = 0;

  /// Transaction number

  /// The fields used in the projection etc.
  field_list_t _field_definition;
  field_name_list_t _named_field_definition;
  field_list_t _indexed_field_definition;

  std::weak_ptr<ResponseTask> _responseTask;

  bool producesPositions = true;

  std::string _planId;
  std::string _operatorId;
  std::string _planOperationName;
  
  tx::TXContext _txContext;

};


}}

#endif  // SRC_LIB_ACCESS_PLANOPERATION_H_
