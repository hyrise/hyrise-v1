#pragma once

#ifdef __SSE__
#  include <immintrin.h>
#endif

#ifdef __SSE__
#  define PAUSE() _mm_pause()
#elif defined(__PPC64__)
   /* see: cpu_relax of linux' arch/powerpc/include/asm/processor.h 
      or ISAv2.07B, Book II, Chapter 3.2 */
#  define HMT_low()    asm volatile("or 1,1,1")
#  define HMT_medium() asm volatile("or 2,2,2")
#  define PAUSE() do { HMT_low(); HMT_medium(); } while (0)
#else
#warning "No good spin-wait instruction known for this platform"
#  define PAUSE()
#endif

#ifdef __SSE__
#  define MEM_FENCE() __builtin_ia32_sfence()
#elif defined(__PPC64__)
#  define MEM_FENCE() asm volatile ("lwsync" : : :"memory");
#else
#warning "No memory storage fence instruction known for this platform"
#  define MEM_FENCE()
#endif

