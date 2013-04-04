// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_EXPRESSIONSCAN_H_
#define SRC_LIB_ACCESS_EXPRESSIONSCAN_H_

#include "access/PlanOperation.h"
#include "helper/types.h"

namespace hyrise {
namespace access {

class ColumnExpression {
public:
  explicit ColumnExpression(const storage::atable_ptr_t &t);
  virtual ~ColumnExpression();

  virtual void setResult(const storage::atable_ptr_t &result,
                         const storage::field_t column,
                         const storage::pos_t row) const = 0;
  virtual std::string getName() const = 0;
  virtual DataType getType() const = 0;

protected:
  storage::atable_ptr_t _table;
};

class AddExp : public ColumnExpression {
public:
  AddExp(const storage::atable_ptr_t &t,
         const storage::field_t field1,
         const storage::field_t field2);
  virtual ~AddExp();

  void setResult(const storage::atable_ptr_t &result,
                 const storage::field_t column,
                 const storage::pos_t row) const;
  std::string getName() const;
  DataType getType() const;

private:
  storage::field_t _field1;
  storage::field_t _field2;
};

class ExpressionScan : public _PlanOperation {
public:
  virtual ~ExpressionScan();

  virtual void executePlanOperation();
  const std::string vname();
  virtual void setExpression(const std::string &name,
                             ColumnExpression *expression);

private:
  ColumnExpression *_expression;
  std::string _column_name;
};

}
}

#endif  // SRC_LIB_ACCESS_EXPRESSIONSCAN_H_
