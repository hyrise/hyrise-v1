// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "access/expressions/expressionGeneration/generatedExpressions/{{ expression.name }}.h"
#include "access/expressions/ExpressionLibrary.h"
#include "storage/AbstractTable.h"
#include "helper/make_unique.h"

namespace hyrise { namespace access {

namespace {
  bool success = ExpressionLibrary::getInstance().add("{{ expression.callName }}", &{{ expression.name }}::creator);
}

bool {{ expression.name }}::deltaExists() { return _table->size() > _mainVector[0]->size(); }

{{ expression.name }}::{{ expression.name }}(const Json::Value& data) : Expression() {
  for (size_t i = 0; i < NUMBER_OF_COLUMNS; ++i) {
    _columns[i] = data["columns"][(Json::UInt)i].asUInt();
  }

  {% for dataType in expression.dataTypes %}
  _value{{loop.index - 1}} = data["values"][(Json::UInt){{loop.index - 1}}].{{ expression.jsonParseMethods[loop.index - 1] }};
  {% endfor %}
}

std::unique_ptr<Expression> {{ expression.name }}::creator(const Json::Value& data) {
  return make_unique<{{ expression.name }}>(data);
}

void {{ expression.name }}::evaluateMain(pos_list_t *results) {
  //value_id_t is preferable, but cannot take -1 value, which is necessary for getValueIdForValue bug
  int64_t valueIdExtended[NUMBER_OF_COLUMNS] = {0};

  {% for number in range(0,expression.numberOfColumns) %}
    valueIdExtended[{{number}}] = _mainDictionary{{number}}->getValueIdForValue(_value{{number}});
    if (!_mainDictionary{{number}}->valueExists(_value{{number}}))
      {% if expression.operators[number] == '==' %}
        valueIdExtended[{{number}}] = -2;
      {% elif expression.operators[number] == '>' %}
        valueIdExtended[{{number}}] = _mainDictionary{{number}}->getValueIdForValueGreater(_value{{number}}) - 1;
      {% else %}
        valueIdExtended[{{number}}] = _mainDictionary{{number}}->getValueIdForValueSmaller(_value{{number}}) + 1;
      {% endif %}
  {% endfor %}

  for(size_t currentRow = 0; currentRow < _mainVector[0]->size(); ++currentRow) {
    if ( {{expression.evaluationString}} )
      results->push_back(currentRow);
  }
}

void {{ expression.name }}::evaluateDelta(pos_list_t *results) {
  value_id_t valueId;
  size_t deltaOffsetInTable = _mainVector[0]->size();

  for(size_t currentRow = 0; currentRow < _deltaVector[0]->size(); ++currentRow) {
    if ( {{expression.evaluationStringDelta}} )
      results->push_back(currentRow + deltaOffsetInTable);
  }

}

void {{ expression.name }}::setup(const storage::c_atable_ptr_t &table) {
  _table = table;
  for (size_t i = 0; i < NUMBER_OF_COLUMNS; ++i) {
    const auto& avs = table->getAttributeVectors(_columns[i]);

    _mainVector[i] = std::dynamic_pointer_cast<storage::FixedLengthVector<value_id_t>>(avs.at(0).attribute_vector);

    if (deltaExists()) {
      _deltaVector[i] = std::dynamic_pointer_cast<storage::ConcurrentFixedLengthVector<value_id_t>>(avs.at(1).attribute_vector);
    }
  }

  {% for number in range(0,expression.numberOfColumns) %}
    _mainDictionary{{number}} = std::dynamic_pointer_cast<storage::OrderPreservingDictionary<{{expression.dataTypes[number]}}>>(table->dictionaryAt(_columns[{{number}}]));
    _deltaDictionary{{number}} = std::dynamic_pointer_cast<storage::ConcurrentUnorderedDictionary<{{expression.dataTypes[number]}}>>(table->dictionaryAt(_columns[{{number}}], _mainVector[0]->size()));
  {% endfor %}

}

}}