#ifndef SRC_LIB_ACCESS_INSERTSCAN_H_
#define SRC_LIB_ACCESS_INSERTSCAN_H_

#include "PlanOperation.h"

/*
  @brief The InsertScan allows to insert a single row into the database

  The idea of this scan is to create a plan operation that allows to
  insert a new row to the database. To achieve this we have to cope
  with different problems.

  @li Make sure that the data is persisted on disk as well
  @li Make sure the data is correctly distributed along all container

*/
class InsertScan : public _PlanOperation {
  AbstractTable::SharedTablePtr data;

 public:


  InsertScan() : _PlanOperation() {
    
  }

  virtual ~InsertScan();

  /*
    Sets the input data for this scan operation. This input data will
    then be distributed over all containers.

    @param data the row data - materialized
  */
  void setInputData(AbstractTable::SharedTablePtr c);

  /*
    Execute this planoperation.

    The strategy is the following: The method fetches a all containers
    from the table and distributes the data according to the
    distribution of the tables.

    @return the input data of the operation.
  */
  virtual void executePlanOperation();

  const std::string vname() {
    return "InsertScan";
  }
};
#endif  // SRC_LIB_ACCESS_INSERTSCAN_H_

