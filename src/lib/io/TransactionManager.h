#ifndef SRC_LIB_IO_TRANSACTIONMANAGER_H_
#define SRC_LIB_IO_TRANSACTIONMANAGER_H_

#include <atomic>

#include "helper/types.h"

namespace hyrise {
namespace tx {

/// Basic transaction manager
class TransactionManager {
  static std::atomic<transaction_id_t> _transactionCount;
 public:
  static TransactionManager& getInstance();
  transaction_id_t getTransactionId();
  void reset();
};

}}

#endif
