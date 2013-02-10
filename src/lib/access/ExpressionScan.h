// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_EXPRESSIONSCAN_H_
#define SRC_LIB_ACCESS_EXPRESSIONSCAN_H_

#include "access/PlanOperation.h"
#include "helper/types.h"

namespace hyrise {
namespace access {

class ColumnExpression {
public:
  explicit ColumnExpression(storage::atable_ptr_t t);
  virtual ~ColumnExpression();

  virtual void setResult(const storage::atable_ptr_t result,
                         const storage::field_t column,
                         const storage::pos_t row) const = 0;
  virtual std::string getName() const = 0;
  virtual DataType getType() const = 0;

protected:
  AbstractTable::SharedTablePtr _table;
};

class AddExp : public ColumnExpression {
public:
  AddExp(storage::atable_ptr_t t,
         const storage::field_t field1,
         const storage::field_t field2);
  virtual ~AddExp();

  virtual void setResult(const storage::atable_ptr_t result,
                         const storage::field_t column,
                         const storage::pos_t row) const;
  virtual std::string getName() const;
  virtual DataType getType() const;

protected:
  storage::field_t _field1;
  storage::field_t _field2;
};

class ExpressionScan : public _PlanOperation {
public:
  ExpressionScan();
  virtual ~ExpressionScan();

  virtual void setExpression(const std::string &name,
                             ColumnExpression *expression);
  virtual void executePlanOperation();
  const std::string vname();

private:
  ColumnExpression *_expression;
  std::string _column_name;
};

}
}

#endif  // SRC_LIB_ACCESS_EXPRESSIONSCAN_H_
