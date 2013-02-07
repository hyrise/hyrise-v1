#ifndef HYRISE_CALIBRATOR_TRACE
#define HYRISE_CALIBRATOR_TRACE

#include <stdlib.h>
#include <sys/time.h>
#ifdef USE_PAPI

#include <papi.h>

#else

typedef long long long_long;

#endif

struct tracer
{
    
#ifdef USE_PAPI
    int Events[1];
#endif
    unsigned long _speed;
    long_long* _v;

    tracer(long_long *v, unsigned long speed=0): _v(v), _speed(speed)
    {
#ifdef USE_PAPI
        Events[0] = PAPI_TOT_CYC;
        PAPI_start_counters(Events, 1);
#else
        timeval t;
        gettimeofday(&t, NULL);
        *_v =  t.tv_sec * 1000000 + t.tv_usec;
#endif
    }

    ~tracer()
    {
#ifdef USE_PAPI
        long_long vs[1];
        PAPI_stop_counters(vs, 1);
        *_v = vs[0];
#else
        timeval t;
        gettimeofday(&t, NULL);
        *_v = (t.tv_sec * 1000000 + t.tv_usec - *_v) * _speed / 1000000;
#endif
    }
};



#endif
