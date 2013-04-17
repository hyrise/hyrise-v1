// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/RadixJoin.h"

#include "access/BasicParser.h"
#include "access/QueryParser.h"

namespace hyrise {
namespace access {

namespace {
  auto _ = QueryParser::registerPlanOperation<RadixJoin>("RadixJoin");
}

void RadixJoin::executePlanOperation() {
}

std::shared_ptr<_PlanOperation> RadixJoin::parse(Json::Value &data) {
  auto instance = BasicParser<RadixJoin>::parse(data);
  instance->setBits1(data["bits1"].asUInt());
  instance->setBits2(data["bits2"].asUInt());
  return instance;
}

const std::string RadixJoin::vname() {
  return "RadixJoin";
}

void RadixJoin::setBits1(const uint32_t b) {
  _bits1 = b;
}

void RadixJoin::setBits2(const uint32_t b) {
  _bits2 = b;
}

uint32_t RadixJoin::bits1() const {
  return _bits1;
}

uint32_t RadixJoin::bits2() const {
  return _bits2;
}

}
}
