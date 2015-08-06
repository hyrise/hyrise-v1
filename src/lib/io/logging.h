#ifndef SRC_LIB_IO_LOGGING_H
#define SRC_LIB_IO_LOGGING_H

#include "io/BufferedLogger.h"
#include "io/NoLogger.h"
#include "io/SimpleLogger.h"

namespace hyrise {
namespace io {

#ifdef PERSISTENCY_NONE
typedef NoLogger Logger;
#endif


#ifdef PERSISTENCY_SIMPLELOGGER
typedef SimpleLogger Logger;
#endif

#ifdef PERSISTENCY_BUFFEREDLOGGER
typedef BufferedLogger Logger;
#endif
}
}

#endif  // SRC_LIB_IO_LOGGING_H
