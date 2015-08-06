// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_UPDATESCAN_H_
#define SRC_LIB_ACCESS_UPDATESCAN_H_

#include "access/system/PlanOperation.h"
#include "access/expressions/pred_SimpleExpression.h"

#include "helper/types.h"

namespace hyrise {
namespace access {

#ifndef UPDATE_SCAN_INSERT_ONLY_H

class UpdateFun {
 public:
  explicit UpdateFun(const storage::atable_ptr_t& t) : _table(t) {};
  virtual ~UpdateFun() {}
  virtual void updateRow(size_t row) {}

 protected:
  storage::atable_ptr_t _table;
};

template <typename T>
class AddUpdateFun : public UpdateFun {
 public:
  AddUpdateFun(const storage::atable_ptr_t& t, const storage::field_t f, const T& val)
      : UpdateFun(t), _column(f), _value(val) {}
  ~AddUpdateFun() {}
  void updateRow(const size_t row) { _table->setValue<T>(_column, row, _table->getValue<T>(_column, row) + _value); }

 private:
  const field_t _column;
  const T _value;
};

template <typename T>
class ColumnSetFun : public UpdateFun {
 public:
  ColumnSetFun(const storage::atable_ptr_t& t, const field_t f, const T& val)
      : UpdateFun(t), _column(f), _value(val) {};
  ~ColumnSetFun() {};
  void updateRow(const size_t row) { _table->setValue<T>(_column, row, _value); }

 private:
  field_t _column;
  T _value;
};

#endif  // UPDATE_SCAN_INSERT_ONLY_H

class UpdateScan : public PlanOperation {
 public:
  UpdateScan();
  virtual ~UpdateScan();

  void executePlanOperation();
  const std::string vname();
  void setUpdateTable(const storage::atable_ptr_t& c);
  void setUpdateFunction(UpdateFun* f);
  void setPredicate(SimpleExpression* e);

 private:
  SimpleExpression* _comparator;
  storage::atable_ptr_t _data;
  UpdateFun* _func;
};
}
}

#endif  // SRC_LIB_ACCESS_UPDATESCAN_H_
