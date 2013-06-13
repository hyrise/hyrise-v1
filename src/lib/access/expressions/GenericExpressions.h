// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_EXPRESSIONS_GENERICEXPRESSIONS_H_
#define SRC_LIB_ACCESS_EXPRESSIONS_GENERICEXPRESSIONS_H_

#include "json.h"

#include "access/expressions/AbstractExpression.h"
#include "storage/FixedLengthVector.h"
#include "storage/Store.h"
#include "storage/storage_types.h"

#include "helper/types.h"

namespace hyrise { namespace access {

// Store, FixedLengthVector, three fields anded
class Store_FLV_F1_EQ_INT_AND_F2_EQ_INT_AND_F3_EQ_INT : public AbstractExpression {

	using VectorType = FixedLengthVector<value_id_t>;
	using SharedVectorType = std::shared_ptr<VectorType>;

	hyrise::storage::c_store_ptr_t _store;

	std::vector<SharedVectorType> _vector_f1;
	std::vector<SharedVectorType> _vector_f2;
	std::vector<SharedVectorType> _vector_f3;

	std::vector<size_t> _of_f1;
	std::vector<size_t> _of_f2;
	std::vector<size_t> _of_f3;
	
	field_t f1;
	field_t f2;
	field_t f3;

	hyrise_int_t v1;
	hyrise_int_t v2;
	hyrise_int_t v3;

public:

	Store_FLV_F1_EQ_INT_AND_F2_EQ_INT_AND_F3_EQ_INT();


	virtual pos_list_t* match(const size_t start, const size_t stop);

  virtual void walk(const std::vector<hyrise::storage::c_atable_ptr_t> &l);

	static std::unique_ptr<Store_FLV_F1_EQ_INT_AND_F2_EQ_INT_AND_F3_EQ_INT> parse(const Json::Value& data);



};

}}

#endif // SRC_LIB_ACCESS_EXPRESSIONS_GENERICEXPRESSIONS_H_