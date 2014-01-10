// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_EXPRESSIONS_GENERICEXPRESSIONS_H_
#define SRC_LIB_ACCESS_EXPRESSIONS_GENERICEXPRESSIONS_H_

#include "json.h"

#include "access/expressions/AbstractExpression.h"
#include "access/expressions/GenericExpressions.inc"
#include "storage/FixedLengthVector.h"
#include "storage/Store.h"
#include "storage/storage_types.h"

#include "helper/types.h"
#include "helper/make_unique.h"
#include "helper/checked_cast.h"
#include "helper/vector_helpers.h"


namespace hyrise { namespace access {

/*
 * Defining Generic Expressions
 *
 * This module is the place to define custom expressions as you need them for
 * query execution in the TableScan plan operation. Expressions are basically
 * a class that provides a match method that is called for every row in the
 * source tables.
 *
 * The two most important macros are:
 *   - DEFINE_EXPRESSION_CLASS( name, field_sequence, logic_sequence)
 *   - REGISTER_EXPRESSION_CLASS( name )
 *
 * The first defines and implements the ncessary methods for a given field
 * definitino. A field definition is a sequnce in terms of C++ macros and has
 * exactly 4 values. The field name, the type, the comparison operator and the
 * method name used to extract from json.
 *
 * An example for such a sequence is:
 *
 *     (f1)(hyrise_int_t)(==)(asUInt)
 *
 * The parentheses are extremeley important. Now the paremter to the define
 * macro is bascially a sequence of sequences so it must be defined as
 *
 *     (seq1...)(seq2...)(seqn..)
 *
 * The logical operators are defined in the same way as a sequence of operations:
 *
 *     ()(&&)(||)
 *
 * The first parameter is always empty and used as a placeholder in the code
 * generation. Please keep the evaluation order and operator precedence in
 * mind.
 */
 struct GenericExpressionsHelper{
 };

#define STORE_TWO_FIELD_SEQ_FLD1_LTE_FLOAT (f1)(hyrise_float_t)(<=)(asFloat)
#define STORE_TWO_FIELD_SEQ_FLD1_LTE_INT (f1)(hyrise_int_t)(<=)(asInt)
#define STORE_TWO_FIELD_SEQ_FLD2_GTE_FLOAT (f2)(hyrise_float_t)(>=)(asFloat)
#define STORE_TWO_FIELD_SEQ_FLD2_GTE_INT (f2)(hyrise_int_t)(>=)(asInt)

#define STORE_TWO_FIELD_SEQ_FLOAT_BTW (STORE_TWO_FIELD_SEQ_FLD1_LTE_FLOAT)(STORE_TWO_FIELD_SEQ_FLD2_GTE_FLOAT)
#define STORE_TWO_FIELD_SEQ_INT_BTW (STORE_TWO_FIELD_SEQ_FLD1_LTE_INT)(STORE_TWO_FIELD_SEQ_FLD2_GTE_INT)

DEFINE_EXPRESSION_CLASS(STORE_FLV_F1_LTEQ_FLOAT_AND_F2_GTEQ_FLOAT, STORE_TWO_FIELD_SEQ_FLOAT_BTW, ()(&&));
DEFINE_EXPRESSION_CLASS(STORE_FLV_F1_LTEQ_INT_AND_F2_GTEQ_INT, STORE_TWO_FIELD_SEQ_INT_BTW, ()(&&));

#define FLD_1 (f1)(hyrise_int_t)(==)(asUInt64)
#define FLD_2 (f2)(hyrise_int_t)(==)(asUInt64)
#define FLD_3 (f3)(hyrise_int_t)(==)(asUInt64)

#define STORE_THREE_FIELD_SEQ (FLD_1)(FLD_2)(FLD_3)

#define STORE_THREE_FIELD_LOG ()(&&)(&&)

DEFINE_EXPRESSION_CLASS(Store_FLV_F1_EQ_INT_AND_F2_EQ_INT_AND_F3_EQ_INT, STORE_THREE_FIELD_SEQ, STORE_THREE_FIELD_LOG);



DEFINE_EXPRESSION_CLASS(Store_FLV_F1_EQ_INT_AND_F2_EQ_INT_AND_F3_EQ_INT_AND_F4_GTE_INT_AND_F5_LTE_INT,
  ((f1)(hyrise_int_t)(==)(asUInt64))((f2)(hyrise_int_t)(==)(asUInt64))((f3)(hyrise_int_t)(==)(asUInt64))((f4)(hyrise_int_t)(>=)(asUInt64))((f5)(hyrise_int_t)(<=)(asUInt64)),
  ()(&&)(&&)(&&)(&&));


#define STORE_ONE_FIELD_SEQ ((f1)(hyrise_int_t)(==)(asUInt64))
DEFINE_EXPRESSION_CLASS(Store_FLV_F1_EQ_INT, STORE_ONE_FIELD_SEQ, ());
DEFINE_EXPRESSION_CLASS(Store_FLV_F1_EQ_STRING, ((f1)(hyrise_string_t)(==)(asString)), ());
DEFINE_EXPRESSION_CLASS(Store_FLV_F1_EQ_STRING_OR_F2_NEQ_FLOAT, ((f1)(hyrise_string_t)(==)(asString))((f2)(hyrise_float_t)(!=)(asFloat)), ()(||));
DEFINE_EXPRESSION_CLASS(Store_FLV_F1_EQ_INT32_OR_F2_NEQ_FLOAT, ((f1)(hyrise_int32_t)(==)(asInt))((f2)(hyrise_float_t)(!=)(asFloat)), ()(||));


#define STORE_TWO_FIELD_SEQ_FLD1 (f1)(hyrise_int_t)(==)(asUInt64)
#define STORE_TWO_FIELD_SEQ_FLD2 (f2)(hyrise_int_t)(==)(asUInt64)
#define STORE_TWO_FIELD_SEQ (STORE_TWO_FIELD_SEQ_FLD1)(STORE_TWO_FIELD_SEQ_FLD2)

DEFINE_EXPRESSION_CLASS(Store_FLV_F1_EQ_INT_AND_F2_EQ_INT, STORE_TWO_FIELD_SEQ, ()(&&));

}}

#endif // SRC_LIB_ACCESS_EXPRESSIONS_GENERICEXPRESSIONS_H_
