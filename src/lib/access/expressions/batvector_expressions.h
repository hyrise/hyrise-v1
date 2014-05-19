#pragma once
// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "json.h"

#include "access/expressions/AbstractExpression.h"

#include "storage/AbstractTable.h"
#include "storage/Store.h"
#include "storage/FixedLengthVector.h"
#include "storage/storage_types.h"


#include "helper/types.h"
#include "helper/make_unique.h"
#include "helper/checked_cast.h"

#include "storage/bat_vector.h"

namespace hyrise { namespace access {


  /*
  * Base Class for all exressions that we use
  */
  template<typename Op>
  class BatVectorExpression : public AbstractExpression {

    storage::c_atable_ptr_t _table;
    std::shared_ptr<storage::BATVector<value_id_t>> _bat;

    const field_t _field = 0;
    const hyrise_int_t _value;

    // Marker
    value_id_t _vid;

  public:

    BatVectorExpression(const field_t f, const hyrise_int_t v): _field(f), _value(v) {
    }

    virtual pos_list_t* match(const size_t start, const size_t stop) {

      if (_value != _bat->getDefaultValue()) {
        auto result = _bat->template match<Op>(start, stop, _vid);
        return new pos_list_t(result);
      } else {
        // We have to populate the pos list from the bit vector
        auto result = _bat->anti_match(start, stop);
        return new pos_list_t(result);
      }

      return nullptr;
    }

    // Extract the tables and columns we need
    virtual void walk(const std::vector<storage::c_atable_ptr_t>& l) {
      auto store = checked_pointer_cast<const storage::Store>(l.front());
      const auto& avs = store->getAttributeVectors(_field);
      const auto& av = avs[0];
      _bat = checked_pointer_cast<storage::BATVector<value_id_t, true>>(av.attribute_vector);
      _vid = l.front()->getValueIdForValue(_field, _value).valueId;
    }

    /*
    * Parse the JSON op
    */
    static std::unique_ptr<BatVectorExpression> parse(const Json::Value& data) {
      return make_unique<BatVectorExpression>(data["column"].asUInt(), data["value"].asInt());
    }

  };


}}