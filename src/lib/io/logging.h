#ifndef SRC_LIB_IO_LOGGING_H
#define SRC_LIB_IO_LOGGING_H

#include "io/BufferedLogger.h"
#include "io/NoLogger.h"
#include "io/SimpleLogger.h"

namespace hyrise {
namespace io {

#if !defined(PERSISTENCY) || PERSISTENCY == NONE
  typedef NoLogger Logger;
#elif PERSISTENCY == SIMPLE
  typedef SimpleLogger Logger;
#else // BUFFERED
  typedef BufferedLogger Logger;
#endif

}
}

#endif // SRC_LIB_IO_LOGGING_H
