#ifndef SRC_LIB_IO_SIMPLELOGGER_H
#define SRC_LIB_IO_SIMPLELOGGER_H

#include <sstream>
#include <mutex>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "storage/storage_types.h"
#include "helper/types.h"

namespace hyrise {
namespace io {

class SimpleLogger {
public:
  SimpleLogger(const SimpleLogger&) = delete;
  SimpleLogger &operator=(const SimpleLogger&) = delete;

  static SimpleLogger &getInstance();

  template <typename T>
  void logDictionary(storage::table_id_t table_id,
                     storage::field_t column,
                     const T &value,
                     storage::value_id_t value_id);
  void logValue(tx::transaction_id_t transaction_id,
                storage::table_id_t table_id,
                storage::pos_t row,
                storage::pos_t invalidated_row,
                uint64_t field_bitmask,
                const ValueIdList *value_ids);
  void logCommit(tx::transaction_id_t transaction_id);

private:
  SimpleLogger();

  int _fd;
  std::mutex _mutex;
};

template <typename T>
void SimpleLogger::logDictionary(const storage::table_id_t table_id,
                                 const storage::field_t column,
                                 const T &value,
                                 const storage::value_id_t value_id) {
  std::stringstream ss;
  ss << "(d," << (int)table_id << "," << column << "," << value << "," << value_id << ")";
  _mutex.lock();
  write(_fd, (void *) ss.str().c_str(), ss.str().length());
  fsync(_fd);
  _mutex.unlock();
}

}
}

#endif // SRC_LIB_IO_SIMPLELOGGER_H
