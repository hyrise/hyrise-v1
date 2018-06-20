// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_{{ expression.name.upper() }}_H_
#define SRC_LIB_ACCESS_{{ expression.name.upper() }}_H_

#include "access/expressions/Expression.h"

#define NUMBER_OF_COLUMNS ({{ expression.numberOfColumns }})

namespace hyrise { namespace access {

class {{ expression.name }} : public Expression{
public:
  {{ expression.name }}(const Json::Value& data);
  static std::unique_ptr<Expression> creator(const Json::Value& data);

  void setup(const storage::c_atable_ptr_t &table);

  std::unique_ptr<Expression> clone();
private:
  void evaluateMain(pos_list_t *results, const size_t start = 0, const size_t stop = 0);
  void evaluateDelta(pos_list_t *results);

  std::array<size_t, NUMBER_OF_COLUMNS> _columns;

  bool deltaExists();

  {% for number in range(0, expression.numberOfColumns) %}
  std::shared_ptr<hyrise::storage::OrderPreservingDictionary<{{ expression.dataTypes[number] }}>> _mainDictionary{{number}};
  std::shared_ptr<hyrise::storage::ConcurrentUnorderedDictionary<{{ expression.dataTypes[number] }}>> _deltaDictionary{{number}};
  {{ expression.dataTypes[number] }} _value{{number}};
  {% endfor %}

  std::shared_ptr<hyrise::storage::FixedLengthVector<value_id_t>> _mainVector[NUMBER_OF_COLUMNS];
  std::shared_ptr<hyrise::storage::ConcurrentFixedLengthVector<value_id_t>> _deltaVector[NUMBER_OF_COLUMNS]; 

  std::shared_ptr<const Json::Value> _jsonData;
};

}}

#endif