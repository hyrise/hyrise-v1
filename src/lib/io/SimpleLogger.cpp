#include "io/SimpleLogger.h"

#include <errno.h>
#include <string.h>

#include <helper/Settings.h>

namespace hyrise {
namespace io {

SimpleLogger& SimpleLogger::getInstance() {
  static SimpleLogger instance;
  return instance;
}

void SimpleLogger::logValue(const tx::transaction_id_t transaction_id,
                            const std::string& table_name,
                            const storage::pos_t row,
                            const storage::pos_t invalidated_row,
                            const ValueIdList* value_ids) {
  std::stringstream ss;
  ss << "(v," << (int)transaction_id << "," << table_name << "," << (int)row << "," << (int)invalidated_row << ",(";
  if (value_ids != nullptr) {
    ss << (*value_ids)[0].valueId;
    for (auto it = ++value_ids->cbegin(); it != value_ids->cend(); it++)
      ss << "," << it->valueId;
  }
  ss << "))";
  _mutex.lock();
  write(_fd, (void*)ss.str().c_str(), ss.str().length());
  _mutex.unlock();
}

void SimpleLogger::logCommit(const tx::transaction_id_t transaction_id) {
  std::stringstream ss;
  ss << "(t," << transaction_id << ")";
  _mutex.lock();
  write(_fd, (void*)ss.str().c_str(), ss.str().length());
  _mutex.unlock();
}

void SimpleLogger::flush() { fsync(_fd); }

SimpleLogger::SimpleLogger() {
  std::string baseDirectory = Settings::getInstance()->getLogDir();
  std::string filename = baseDirectory + "log.txt";

  // create log dir if it does not exists
  struct stat buffer;
  if (stat(baseDirectory.c_str(), &buffer) != 0)
    if (mkdir(baseDirectory.c_str(), 0755) != 0 && errno != EEXIST)
      throw std::runtime_error(strerror(errno));

  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

  _fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
  if (_fd == -1)
    printf("Something went wrong while creating the logfile: %s\n", strerror(errno));
}
}
}
