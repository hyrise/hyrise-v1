// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_EXPRESSION_H_
#define SRC_LIB_ACCESS_EXPRESSION_H_

#include <vector>
#include <stdexcept>
#include <iostream>

#include "helper/types.h"
#include "json.h"

#include "storage/storage_types.h"
#include "storage/FixedLengthVector.h"
#include "storage/ConcurrentFixedLengthVector.h"
#include "storage/OrderPreservingDictionary.h"
#include "storage/ConcurrentUnorderedDictionary.h"

#include <array>

namespace hyrise { namespace access {

class Expression {
    friend class PipeliningTableScan;

	public:
		static std::unique_ptr<Expression> creator(const Json::Value& data);

		virtual void setup(const storage::c_atable_ptr_t &table) = 0;

		pos_list_t* evaluate();
		virtual bool deltaExists() = 0;

		virtual std::unique_ptr<Expression> clone() = 0;
	protected:
		storage::c_atable_ptr_t _table;

		virtual void evaluateMain(pos_list_t *results, const size_t start = 0, const size_t stop = 0) = 0;
		virtual void evaluateDelta(pos_list_t *results) = 0;
};

}}

#endif