//without this it does not work on my gentoo ...
#define PACKAGE
#define PACKAGE_VERSION

#include "backward/backward.hpp"

namespace backward {

backward::SignalHandling sh({
    SIGILL,
        SIGABRT,
        SIGFPE,
        SIGSEGV,
        SIGBUS,
        });


} // namespace backward
