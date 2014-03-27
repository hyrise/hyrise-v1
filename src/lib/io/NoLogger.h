#ifndef SRC_LIB_IO_NOLOGGER_H
#define SRC_LIB_IO_NOLOGGER_H

#include "helper/types.h"

namespace hyrise {
namespace io {

class NoLogger {
 public:
  NoLogger(const NoLogger&) = delete;
  NoLogger& operator=(const NoLogger&) = delete;

  static NoLogger& getInstance() {
    static NoLogger instance;
    return instance;
  }

  template <typename T>
  void logDictionary(const std::string& table_name,
                     storage::field_t column,
                     const T& value,
                     storage::value_id_t value_id) {
    // do nothing
  }
  void logValue(tx::transaction_id_t transaction_id,
                const std::string& table_name,
                storage::pos_t row,
                storage::pos_t invalidated_row,
                const ValueIdList* value_ids) {
    // do nothing
  }
  void logCommit(tx::transaction_id_t transaction_id) {
    // do nothing
  }
  void flush() {
    // do nothing
  }

  void restore(const char* logfile = NULL) {
    // do nothing
  }

 private:
  NoLogger() {
    // do nothing
  }
};
}
}

#endif  // SRC_LIB_IO_NOLOGGER_H
