#include "access/Print.h"
#include <iostream>

namespace hyrise {
namespace access {

namespace {
auto _ = QueryParser::registerPlanOperation<Print>("Print");
}

Print::Print() : _limit(0), _comment("") {}

void Print::executePlanOperation() {
  std::cout << "Print Operator: " << _comment << std::endl;
  // print all input data and add it to the output
  for (size_t i = 0; i < input.numberOfTables(); ++i) {
    if (_limit > 0)
      input.getTable(i)->print(_limit);
    else
      input.getTable(i)->print();
    addResult(input.getTable(i));
  }
}

std::shared_ptr<PlanOperation> Print::parse(const Json::Value& data) {
  std::shared_ptr<Print> p = std::make_shared<Print>();
  if (data.isMember("limit"))
    p->setLimit(data["limit"].asInt());
  if (data.isMember("comment"))
    p->setComment(data["comment"].asString());
  return p;
}

void Print::setLimit(size_t limit) { _limit = limit; }

void Print::setComment(std::string comment) { _comment = comment; }
}
}
