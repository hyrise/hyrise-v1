#ifndef SRC_LIB_STORAGE_SERIAL_H_
#define SRC_LIB_STORAGE_SERIAL_H_

#include "AbstractResource.h"

#include <atomic>

/// A Serial class that can be used to define auto_increment columns for
//tables. For each column a serial will be stored in the resource manager
class Serial : public AbstractResource {

  typedef unsigned long serial_t;

  std::atomic<serial_t> _serial;

 public:

  virtual ~Serial() {}

  Serial();

  void reset();

  serial_t next();
};

#endif

