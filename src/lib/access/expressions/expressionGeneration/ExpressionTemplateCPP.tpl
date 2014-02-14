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
  value_id_t valueIds[NUMBER_OF_COLUMNS] = {0};

  {#
  ## This amount of if-else cluttering is necessary because Hyrise does not
  ## return an error or an obvious wrong value like -1 for calls of
  ## getValueIdForValueGreater with a value which does not exist in the dictionary.
  ## Instead it returns std::numeric_limits<value_id_t>::max(). The content of
  ## valueIds[{{number}}] has to be checked in case the operator is not '=='
  ## to deliver the correct results for all expression evaluations
  #}

  {% for number in range(0,expression.numberOfColumns) %}
    {% if expression.operators[number] == '>' %}
  valueIds[{{number}}] = _mainDictionary{{number}}->getValueIdForValueGreater(_value{{number}});
    {% elif expression.operators[number] == '<' %}
  valueIds[{{number}}] = _mainDictionary{{number}}->getValueIdForValueSmaller(_value{{number}});
    {% else %}
  valueIds[{{number}}] = _mainDictionary{{number}}->getValueId(_value{{number}}, false);
      {% if expression.operators[number] != '==' %}
  if (valueIds[{{number}}] == std::numeric_limits<value_id_t>::max())
        {% if expression.operators[number] == '>=' %}
    valueIds[{{number}}] = _mainDictionary{{number}}->getValueIdForValueGreater(_value{{number}});
        {% else %}
    valueIds[{{number}}] = _mainDictionary{{number}}->getValueIdForValueSmaller(_value{{number}});
        {% endif %}
      {% endif %}
    {% endif %}
  {% endfor %}

  size_t mainVectorSize = _mainVector[0]->size();
  for(size_t currentRow = 0; currentRow < mainVectorSize; ++currentRow) {
    if ( {{expression.evaluationString}} )
      results->push_back(currentRow);
  }
}

void {{ expression.name }}::evaluateDelta(pos_list_t *results) {
  size_t deltaOffsetInTable = _mainVector[0]->size();
  {% for number in range(0,expression.numberOfEQComparisons) %}
    value_id_t valueId{{number}};
    valueId{{number}} = _deltaDictionary{{number}}->getValueId(_value{{number}}, false);
  {% endfor %}

  size_t deltaVectorSize = _deltaVector[0]->size();
  for(size_t currentRow = 0; currentRow < deltaVectorSize; ++currentRow) {
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