// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <memory>
#include <cstdint>
#include <unordered_map>

#include "helper/checked_cast.h"
#include "helper/types.h"

#include "storage/bat_vector.h"
#include "storage/AbstractTable.h"
#include "storage/AbstractAttributeVector.h"

namespace hyrise { namespace storage { namespace model {

  using avcb = storage::AbstractTable::abstract_attribute_vector_callback;

  /*
  * This is an extended callback function object that determines the most
  * suitable storage representation for a given table.
  *
  * Using this functor is good for memory but can be bad in terms of time, as it needs to scan each column
  */
  struct AttributeModelCallback {

    const storage::c_atable_ptr_t& tab;

    uint32_t current_attribute = 0;

    AttributeModelCallback(const storage::c_atable_ptr_t& t): tab(t) {
    }


    /*
     * This is the function that is called for the actual implementation of choosing the right storage type
     */
    std::shared_ptr<storage::AbstractAttributeVector> operator()(const size_t width) {

      // First check the distinct value count for this attribute based on the
      // dictionary and then calculate the most frequent value
      const auto distinct = tab->dictionaryAt(current_attribute, 0)->size();

      // If we have too many distinct values, we dont need to check
      if (distinct * 4 > tab->size()) {
        current_attribute++;
        return std::make_shared<storage::FixedLengthVector<value_id_t>>(width, 0);
      }

      // Calculate most frequent value
      auto av = tab->getAttributeVectors(current_attribute).front();
      auto mf = mostFrequent(checked_pointer_cast<storage::FixedLengthVector<value_id_t>>(av.attribute_vector), av.attribute_offset );

      auto mem_flv = 32 * tab->size();
      auto mem_bat = tab->size() / 8.0 + (tab->size() - std::get<1>(mf)) * 2 * 8;

      if (mem_flv < mem_bat ) {
        current_attribute++;
        return std::make_shared<storage::FixedLengthVector<value_id_t>>(width, 0);
      } else {
        current_attribute++;
        return std::make_shared<storage::BATVector<value_id_t, true>>(std::get<0>(mf));
      }


    }


    std::tuple<value_id_t,size_t> mostFrequent(const std::shared_ptr<storage::FixedLengthVector<value_id_t>>& aaa, size_t offset=0) {
      std::unordered_map<value_id_t, size_t> data;
      auto stop = aaa->size();
      for(size_t i=0; i < stop; ++i) {
        const auto val = aaa->get(offset,i);
        if (data.count(val) > 0) {
          data[val]++;
        } else {
          data[val] = 1;
        }
      }

      value_id_t result = 0;
      size_t maxv = 0;
      for(const auto& p : data) {
        maxv = p.second > maxv ? p.second : maxv;
        result = p.first;
      }
      return std::make_tuple(result, maxv);
    }

  };


}}}
