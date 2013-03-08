// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_BARRIER_H_
#define SRC_LIB_ACCESS_BARRIER_H_

#include "BasicParser.h"
#include "PlanOperation.h"
#include "QueryParser.h"

namespace hyrise{ namespace access {

class Barrier : public _PlanOperation {

	static bool is_registered;

public:

	~Barrier(){}

	void executePlanOperation() {
		for(size_t i=0; i < _field_definition.size(); ++i)
			addResult(getInputTable(i));
	}

	const std::string vname() {return "Barrier"; }

	static std::shared_ptr<_PlanOperation> parse(Json::Value &data) {
		return BasicParser<Barrier>::parse(data);
	}

};

}}


#endif // SRC_LIB_ACCESS_BARRIER_H_
