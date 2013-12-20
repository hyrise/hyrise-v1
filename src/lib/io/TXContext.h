// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <helper/types.h>
#include "cereal/cereal.hpp"

namespace hyrise { namespace tx {

struct TXContext {
  using id_t = transaction_id_t;

  transaction_id_t tid = UNKNOWN;
  transaction_id_t lastCid = UNKNOWN;
  transaction_id_t cid = UNKNOWN;

  TXContext() = default;

  TXContext(id_t _tid, id_t _lastCid, id_t _cid = UNKNOWN):
      tid(_tid), lastCid(_lastCid), cid(_cid) {}

  template<class Archive>
  void serialize(Archive & archive) {
    archive(cereal::make_nvp("transaction_id", tid),
            cereal::make_nvp("last_commit_it", lastCid),
            cereal::make_nvp("commit_id", cid));
  }

};

}}
