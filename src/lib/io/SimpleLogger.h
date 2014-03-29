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
  SimpleLogger& operator=(const SimpleLogger&) = delete;

  static SimpleLogger& getInstance();

  template <typename T>
  void logDictionary(const std::string& table_name,
                     storage::field_t column,
                     const T& value,
                     storage::value_id_t value_id);
  void logValue(tx::transaction_id_t transaction_id,
                const std::string& table_name,
                storage::pos_t row,
                storage::pos_t invalidated_row,
                const ValueIdList* value_ids);
  void logCommit(tx::transaction_id_t transaction_id);
  void flush();

 private:
  SimpleLogger();

  int _fd;
  std::mutex _mutex;
};

template <typename T>
void SimpleLogger::logDictionary(const std::string& table_name,
                                 const storage::field_t column,
                                 const T& value,
                                 const storage::value_id_t value_id) {
  std::stringstream ss;
  ss << "(d," << table_name << "," << column << "," << value << "," << value_id << ")";
  _mutex.lock();
  write(_fd, (void*)ss.str().c_str(), ss.str().length());
  _mutex.unlock();
}
}
}

#endif  // SRC_LIB_IO_SIMPLELOGGER_H
