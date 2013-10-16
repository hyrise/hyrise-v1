// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_STOREDPROCEDURES_TPCCSTOCKLEVEL_H_
#define SRC_LIB_ACCESS_STOREDPROCEDURES_TPCCSTOCKLEVEL_H_

#include <helper/types.h>
#include <io/TXContext.h>

#include "helper.h"


namespace hyrise { namespace access {

class TPCCStockLevelProcedure {
 public:
  TPCCStockLevelProcedure(int w_id, int d_id, int threshold);

  result_map_t execute();

 private:
  //illegal constructors
  TPCCStockLevelProcedure();
  TPCCStockLevelProcedure(const TPCCStockLevelProcedure& procedure);

  //queries
  storage::c_atable_ptr_t getOId();
  storage::c_atable_ptr_t getStockCount();

  //transaction stuff
  storage::c_atable_ptr_t loadAndValidate(std::string name, const tx::TXContext& tx);
  tx::TXContext startTransaction();
  void commit(tx::TXContext tx);


  tx::TXContext _tx;

  const int _w_id;
  const int _d_id;
  const int _threshold;
  int _next_o_id;
};

}} // namespace hyrise::access

#endif // SRC_LIB_ACCESS_STOREDPROCEDURES_TPCCSTOCKLEVEL_H_
