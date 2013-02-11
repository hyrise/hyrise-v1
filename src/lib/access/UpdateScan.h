// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_UPDATESCAN_H_
#define SRC_LIB_ACCESS_UPDATESCAN_H_

#include <access/PlanOperation.h>
#include <access/predicates.h>

#include <storage/PointerCalculator.h>
#include <storage/Table.h>


#ifndef UPDATE_SCAN_INSERT_ONLY_H
class UpdateFun {
 protected:
  hyrise::storage::atable_ptr_t table;

 public:
  explicit UpdateFun(hyrise::storage::atable_ptr_t t) : table(t) {};
  virtual ~UpdateFun() {};
  virtual void updateRow(size_t row) {};
};

template < typename T >
class AddUpdateFun : public UpdateFun {
 private:
  field_t column;
  T value;
 public:
  AddUpdateFun(hyrise::storage::atable_ptr_t t, field_t f, T val) : UpdateFun(t), column(f), value(val) {};
  ~AddUpdateFun() {};
  void updateRow(size_t row) {
    table->setValue<T>(column, row, table->getValue<T>(column, row) + value);
  }
};

template < typename T >
class ColumnSetFun : public UpdateFun {
 private:
  field_t column;
  T value;
 public:
  ColumnSetFun(hyrise::storage::atable_ptr_t t, field_t f, T val) : UpdateFun(t), column(f), value(val) {};
  ~ColumnSetFun() {};
  void updateRow(size_t row) {
    table->setValue<T>(column, row, value);
  }
};
#endif


/*
  @brief The UpdateScan allows to update rows in a table
*/
class UpdateScan : public _PlanOperation {
 private:
  SimpleExpression *comparator;
  hyrise::storage::atable_ptr_t data;
  UpdateFun *func;
 public:
  UpdateScan() : _PlanOperation() {
    
    comparator = nullptr;
    func = nullptr;
  }
  virtual ~UpdateScan();

  void setUpdateTable(hyrise::storage::atable_ptr_t c) {
    data = c;
  };
  void setUpdateFunction(UpdateFun *f) {
    func = f;
  };
  void setPredicate(SimpleExpression *e) {
    comparator = e;
  };

  void executePlanOperation();

  const std::string vname() {
    return "UpdateScan";
  }
};
#endif  // SRC_LIB_ACCESS_UPDATESCAN_H_

