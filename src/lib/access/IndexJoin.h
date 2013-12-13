// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_ACCESS_EXPR_INDEX_JOIN_H_
#define SRC_LIB_ACCESS_EXPR_INDEX_JOIN_H_

#include <access/system/PlanOperation.h>

#include "json.h"
#include "helper/types.h"


namespace hyrise { namespace access {

/**
 * The task of the IndexJoin operator is to take a pos list as the
 * input of the left side and perform a look up for each value of each
 * row on the left side in an index specified for the right side. The
 * behavior is much like an HashJoin, but instead of using the hash
 * table for the right side we use an existing index
 */
class IndexJoin : public PlanOperation {

  // Basic Information needed
  std::string _indexName;
  
  const pos_list_t *_tab_pos_list;
  
  storage::c_atable_ptr_t _left;
  storage::c_atable_ptr_t _right;

public:

  static std::shared_ptr<PlanOperation> parse(const Json::Value &data);

  void executePlanOperation();

};

}}

#endif // SRC_LIB_ACCESS_EXPR_INDEX_JOIN_H_
