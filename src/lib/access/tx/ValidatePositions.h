// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_TX_VALIDATEPOSITIONS_H_
#define SRC_LIB_ACCESS_TX_VALIDATEPOSITIONS_H_

#include <access/system/PlanOperation.h>

namespace hyrise { namespace access {

class ValidatePositions : public PlanOperation {

public:

	void executePlanOperation();

	static std::shared_ptr<PlanOperation> parse(const Json::Value &data);

};

}}

#endif // SRC_LIB_ACCESS_TX_VALIDATEPOSITIONS_H_
