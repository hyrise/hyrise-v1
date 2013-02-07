#ifndef SRC_LIB_ACCESS_SPECIALEXPRESSION_H_
#define SRC_LIB_ACCESS_SPECIALEXPRESSION_H_

#include "access/pred_SimpleExpression.h"
#include "helper/types.h"
#include "storage/FixedLengthVector.h"
#include "storage/OrderPreservingDictionary.h"
#include "storage/storage_types.h"

namespace hyrise { namespace access {

class SpecialExpression : public SimpleExpression {
  storage::c_atable_ptr_t _table;
  std::shared_ptr<FixedLengthVector<value_id_t>> _vector;
  std::shared_ptr<OrderPreservingDictionary<hyrise_int_t>> _dict;
  const size_t _column;
  const hyrise_int_t _value;
  value_id_t _valueid;
 public:
  SpecialExpression(const size_t& column, const hyrise_int_t& value);
  inline virtual bool operator()(const size_t& row);
  virtual pos_list_t* match(const size_t start, const size_t stop);
  virtual void walk(const std::vector<hyrise::storage::c_atable_ptr_t> &l);
};

}}

#endif
