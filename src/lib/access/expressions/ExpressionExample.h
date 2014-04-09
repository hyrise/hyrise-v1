// // Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
// #ifndef SRC_LIB_ACCESS_EXPRESSIONEXAMPLE_H_
// #define SRC_LIB_ACCESS_EXPRESSIONEXAMPLE_H_

// #include "access/expressions/Expression.h"

// namespace hyrise {
// namespace access {

// class ExpressionExample : public Expression {
//  public:
//   ExpressionExample(const size_t column, const hyrise_int_t value);
//   static std::unique_ptr<Expression> creator(const Json::Value& data);

//   void setup(const storage::c_atable_ptr_t& table);

//  private:
//   void evaluateMain(pos_list_t* results);
//   void evaluateDelta(pos_list_t* results);

//   hyrise_int_t _value;
//   std::shared_ptr<hyrise::storage::OrderPreservingDictionary<hyrise_int_t>> _mainDictionary;
// 	std::shared_ptr<hyrise::storage::ConcurrentUnorderedDictionary<hyrise_int_t>> _deltaDictionary;
// };
// }
// }

// #endif