// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SCRIPTOPERATION_H_
#define SRC_LIB_ACCESS_SCRIPTOPERATION_H_

#include <unordered_map>

#include "access/system/PlanOperation.h"
#include "json.h"

#ifdef WITH_V8
#include <v8.h>
#include <v8-profiler.h>
#endif

namespace hyrise {
namespace access {

#ifdef WITH_V8
// This is the context data that we use in the isolate data per process.
// Basically it contains a map of all available tables to the plan operation
// with given keys. The key is the offset in this table. The first tables in
// this list are always the input tables of the plan operation
struct IsolateContextData {
  std::vector<storage::c_atable_ptr_t> tables;
  bool recordPerformance;
  std::string papiEvent;
  Json::Value jsonQueryPerformanceValues;
  Json::Value jsonQueryDataflow;
};

void TableFilter(const v8::FunctionCallbackInfo<v8::Value>& args);
void convertDataflowDataToJson(v8::Isolate* isolate, size_t internalId, size_t cardinality, Json::Value& result);
void convertPerformanceDataToJson(v8::Isolate* isolate,
                                  performance_vector_t& perfDataVector,
                                  epoch_t queryStart,
                                  Json::Value& result);

#endif

/// This is a simple scriptable plan operation that allows to write the
/// operation itself in JavaScript instead of C++ for rapid prototyping
class ScriptOperation : public PlanOperation {

 private:
  // The Script Name to execute
  std::string _scriptName;
  std::string _scriptSource;
  std::string _papiEvent;
  std::vector<std::string> _parameters;

 public:
  ScriptOperation();

  void executePlanOperation();
  virtual const PlanOperation* execute();

  static std::shared_ptr<PlanOperation> parse(const Json::Value& data);

  void setParameters(const std::vector<std::string>& params);
  inline void setScriptName(const std::string& n) { _scriptName = n; }
  void setScriptSource(const std::string& source);
  void setPapiEvent(const std::string& papiEvent);

  Json::Value getSubQueryPerformanceData();
  Json::Value getSubQueryDataflow();

 private:
#ifdef WITH_V8
  // Convert the input to an JS Array
  v8::Handle<v8::Array> prepareInputs(v8::Isolate* isolate);

  void createResultHelpers(v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> global);
  IsolateContextData _isoContext;

#endif
};
}
}

#endif /* SRC_LIB_ACCESS_SCRIPTOPERATION_H_ */
