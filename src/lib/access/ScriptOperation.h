// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_SCRIPTOPERATION_H_
#define SRC_LIB_ACCESS_SCRIPTOPERATION_H_

#include <unordered_map>

#include "access/system/PlanOperation.h"

#ifdef WITH_V8
#include <v8.h>
#endif

namespace hyrise {
namespace access {

/// This is a simple scriptable plan operation that allows to write the
/// operation itself in JavaScript instead of C++ for rapid prototyping
class ScriptOperation : public PlanOperation {

private:

  // The Script Name to execute
  std::string _scriptName;

  std::unordered_map<std::string, std::string> _parameters;

public:
  
  ScriptOperation();
  
  void executePlanOperation();

  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);

  inline void setScriptName(std::string n) { _scriptName = n; }

private:

#ifdef WITH_V8
  // Convert the input to an JS Array
  v8::Handle<v8::Array> prepareInputs();

  v8::Handle<v8::Object> prepareParameters();

  void createResultHelpers(v8::Persistent<v8::Context> &context);
#endif
  
};

}
}

#endif /* SRC_LIB_ACCESS_SCRIPTOPERATION_H_ */
