// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#pragma once

#include "access/PipelineObserver.h"
#include "access/PipelineEmitter.h"

#include "helper/types.h"

namespace hyrise {
namespace access {

/*
 * Derive from this class to be able to emit chunks to succeeding PipelineObservers.
 * See docs in "Implementation Details" > "Pipelining"
 */
template <typename U>
class PipelineEmitter {
 protected:
  virtual void emitChunk(storage::c_aresource_ptr_t chunk) {
    auto observers = static_cast<U*>(this)->template getAllSuccessorsOf<AbstractPipelineObserver>();
    if (observers.size() == 0) {
      throw std::runtime_error("Did you forget a PipelineUnion/PipelineMergeHashTables?");
    }
    for (auto& obs : observers) {
      obs->notifyNewChunk(chunk);
    }
  }

  size_t _chunkSize = 100000;
};
}
}
