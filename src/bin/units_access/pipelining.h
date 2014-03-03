// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include "access/PipelineEmitter.h"
#include "io/shortcuts.h"
#include "storage/TableRangeView.h"
#include "access/system/PlanOperation.h"
#include "helper/types.h"


namespace hyrise {
namespace access {

class TestEmitter : public PlanOperation, public PipelineEmitter<TestEmitter> {
 public:
  size_t getNumChunks() {
    return _table->size();
  };

 protected:
  virtual void setupPlanOperation() override {
    _table = io::Loader::shortcuts::load("test/tables/companies.tbl");
  };
  virtual void executePlanOperation() override {
    for (size_t i = 0; i < getNumChunks(); ++i) {
      auto tbl = std::make_shared<storage::TableRangeView>(_table, i, i + 1);
      emitChunk(tbl);
    }
  };

 private:
  storage::c_atable_ptr_t _table;
};
}
}
