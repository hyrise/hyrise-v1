// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_EXPR_PCSCAN_H_
#define SRC_LIB_ACCESS_EXPR_PCSCAN_H_

#include <functional>

#include "json.h"

#include "access/expressions/AbstractExpression.h"
#include "helper/types.h"
#include "storage/PointerCalculator.h"
#include "storage/storage_types.h"
#include "helper/make_unique.h"

namespace hyrise { namespace access {

/*
 * Simple class that allows to compare a single field value with one
 * type and one operator. The operator can be freely choosen from the
 * STD Lib functional types and the Value Type should be one of the
 * HYRISE types
*/
template<typename ValueType, typename Operator=std::equal_to<ValueType> >
class PCScan_F1_OP_TYPE : public AbstractExpression {

  using TableType = storage::AbstractTable;

  field_t _f0;
  ValueType _v0;

  std::shared_ptr<const TableType> _table;
                                                
  // The Operator implementation
  Operator op;

  // Pos List of the other table
  const pos_list_t *_tab_pos_list = nullptr;

 public:

  virtual pos_list_t* match(const size_t start, const size_t stop) {
    auto pl = new pos_list_t;
    auto size = _tab_pos_list->size();
    auto lower = start;
    auto upper = size > stop ? stop : size;

    for(size_t r=lower; r < upper; ++r) {
      auto curr_pos = (*_tab_pos_list)[r];
      if (op(_table->getValue<hyrise_int_t>(_f0, curr_pos), _v0)) 
        pl->push_back(r);
    }
    return pl;
  }

  virtual void walk(const std::vector<hyrise::storage::c_atable_ptr_t> &l) {
    auto tmp = std::dynamic_pointer_cast<const storage::PointerCalculator>(l[0]);
    _table = tmp->getActualTable();
    _tab_pos_list = tmp->getPositions();
  }

  static std::unique_ptr<PCScan_F1_OP_TYPE> parse(const Json::Value& data) {
    auto res = make_unique<PCScan_F1_OP_TYPE>();
    res->_f0 = data["f1"].asUInt();
    res->_v0 = data["v_f1"].asInt();
    return res;
  }

};

}}

#endif
