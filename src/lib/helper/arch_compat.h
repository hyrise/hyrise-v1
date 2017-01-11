#pragma once

#ifdef __SSE__
#  include <immintrin.h>
#  define PAUSE() _mm_pause()
#else
#  define PAUSE()
#endif

#ifdef __powerpc64__
#  define __builtin_ia32_sfence()
#endif
