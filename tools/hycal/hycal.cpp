
#include "hycal.h"

#ifdef USE_SSE
#include<smmintrin.h>
#endif

#include <math.h>
#include <iostream>
#include <pthread.h>

#define ONE t = t->n;
#define TEN ONE ONE ONE ONE ONE ONE ONE ONE ONE ONE
#define HUNDRED TEN TEN TEN TEN TEN TEN TEN TEN TEN TEN                

#define FILL t++; t--; t++; t--; t++; t--; t++; t--; t++; t--;
#define EXEC t = t->n; \
    FILL FILL FILL FILL FILL FILL FILL FILL FILL FILL


#define DEBUG(msg) if(debug) { std::cout << msg << std::endl; fp << msg << std::endl; }
#define FDEBUG(msg) if(debug) { fp << msg << std::endl; }

HyriseCalibrator::~HyriseCalibrator()
{

    fp.close();
    freeMemory();
}

HyriseCalibrator::HyriseCalibrator(size_t region, ulng speed):
    __data(0), _region_size(region), _speed(speed), _max_stride(0)
{
    //
    test_s = sizeof(test_t);
    allocateMemory();
 
    fp.open("hyrise_calibrator_out.txt");
}

void HyriseCalibrator::allocateMemory()
{
    // The space to allocate is the region size divided by the size
    // of test_t
    int t = posix_memalign((void**) &__data, 4096, _region_size);
    if (t != 0)
    {
        std::cout << "Memory allocation failed" << std::endl;
        exit(1);
    }
}

void HyriseCalibrator::freeMemory()
{
    if (__data != NULL)
        free(__data);

    __data = NULL;
}


/*
 * Sequentially fills the region
 *
 * This method sequentially fills a certain data region with chained
 * elements. The stride parameter allows to manipulate the next
 * chained element to create a certain distance.
 *
 * @param size_t s region seize in bytes
 * @param size_t stride the stride in bytes between two elements
 */
void HyriseCalibrator::fillRegion(size_t s, size_t stride)
{
    size_t actual = s == 0 ?  _region_size / test_s : s / test_s;
    
    for (size_t i=1; i < actual; ++i)
    {
        __data[i-1].n = &__data[i];
        __data[i].n = NULL;
    }

    // Circle
    __data[actual-1].n = &__data[0];

    if (stride / test_s > 1)
    {

        size_t i=0, next=0, c=0;
        
        do {
            next += (stride / test_s) % actual;
            __data[i].n = &__data[next];
            i = next;

        } while (i < (actual- (stride / test_s)));
        
        // Circle
        __data[i].n = &__data[0];
    }
}

/*
 * Fills the region with random pointers
 *
 * @see fillRegion()
 */
void HyriseCalibrator::fillRegionRandom(size_t s, size_t stride)
{
    size_t actual = s == 0 ?  _region_size / test_s : s / test_s;
    vector<size_t> list;
    size_t last;
    
    if (stride / test_s  == 1)
    {
        for (size_t i=0; i < actual; ++i)
            list.push_back(i);
    } else {

        for (size_t i=0; i < actual; i += stride / test_s )
            list.push_back(i);
    }

    std::random_shuffle(list.begin(), list.end());
    
    for (size_t i=1; i < list.size(); ++i)
    {
        __data[list[i-1]].n = &__data[list[i]];
        __data[list[i]].n = NULL;
    }

    // Circle
    __data[list.back()].n = &__data[list[0]];


    
}


/*
 * Wrapper for loading
 *
 * This method performas the read on the given range and stride. The
 * delay parameter deterimens if certain dummy noops should be
 * executed before iterating to the next pointer.
 *
 * @param ulng range
 * @param ulng stride
 * @param int dealy
 */
HyriseCalibrator::ulng HyriseCalibrator::perform(ulng range, ulng stride, int delay)
{
    test_t * t, * s;
    
    long_long best = 2000000000, last = 0, tmp = 0;
    double diff;
    ulng inc, iter;

    inc = 1;

    t = __data;
    s = __data;
    

    for (size_t i=0; i < HyriseCalibratorConstants::repeat; ++i)
    {
        iter = inc * HyriseCalibratorConstants::loads;

        if (delay == 1)
        {
            tracer trc(&tmp, _speed);
            while (iter > 0)
            {
                EXEC;
                iter -= 1;
            }
                        
        } else {

            tracer trc(&tmp, _speed);
            while (iter > 0)
            {
                HUNDRED;
                iter -= 100;
            }

        } 
        __dummy += (long) t;
        __dummy += (long) s;

        if (tmp < HyriseCalibratorConstants::mintime)
        {
            inc *= 2;
            --i;
            
        } else {
            tmp /= (inc * HyriseCalibratorConstants::loads);

            if (last != 0)
            {
                diff = fabs(last -tmp)/tmp;
                if (diff > HyriseCalibratorConstants::eps)
                    --i;
                else if (tmp < best)
                    best = tmp;
            }
            last = tmp;
            
            FDEBUG(range << " " << stride << " " << delay << " " << tmp << " " << (inc * HyriseCalibratorConstants::loads));
            

        }
    }

    return best;
}


/*
 * Performs the sequential traversal experiment
 *
 * This experiment basically only sequentially traverses an increasing
 * working set size. The working set size is increased by the power of
 * two values and iterated multiple times.
 */
HyriseCalibrator::intermediate HyriseCalibrator::traversal(ulng stride, bool random, int delay)
{
    assert(stride > 0);
    ulng ctr, step=0, tmp_step;
    ulng range;


    if (debug)
        std::cerr << "# Working Set, time, cycles with stride " << stride << " " << test_s << std::endl;

    // Initialize intermediate
    intermediate i_res;

    // Set the stride
    i_res.stride = stride;
    
    for (ulng r = 1024; r <= _region_size; r *=2)
    {
        for (unsigned i=3; i <=5; ++i)
        {

            // Woha MonetDB Calibrator thanks
            range = (ulng) (r * 0.25 * i);

            if (range > _region_size)
                continue;

            if (stride <= (range / 2))
            {
                // Fill the region with pointer list
                if (!random)
                    fillRegion(range, stride);
                else
                    fillRegionRandom(range, stride);


                i_res.x.push_back(range);
                i_res.y.push_back( perform(range, stride, delay) );

            } else {
                i_res.x.push_back(0);
                i_res.y.push_back(0);
            }
        }
    }
    
    // Add the intermediate result to the list
    return i_res;
}

/*
 * Combined Traversal of the region
 *
 * In this method, the memory region is traversed with varying
 * stride and actual size
 */
void HyriseCalibrator::combinedTraversal(bool random, int delay)
{
    ulng min_stride = test_s; // size of test_t

    for (ulng stride = _max_stride; stride >= min_stride; stride /= 2)
    {
        _results.push_back(traversal(stride,
                                     random, 
                                     delay 
                               ));
    }
}

/*
 * Determining the maximum usefull stride
 *
 * This method basically copies the behavior from the MonetDB
 * calibrator to determine the maximum usefull stride. The algorithm
 * is based on the fact that given a certain stride always a new cache
 * line is touched ( including prefetching) and thus normalized to the
 * accessed data the measured delta time between two different stride
 * types is less than e = 0.1.
 */
HyriseCalibrator::ulng HyriseCalibrator::determineMaxStride()
{
    std::cerr << "========================================" << std::endl;
    std::cerr << "Determining Maximum Stride" << std::endl;
    long last = 0, time = 0;
    ulng stride = test_s / 2, max_stride = _region_size / 2;
    
    do
    {
        stride *= 2;
        fillRegion(_region_size, stride);
        
        last = time;
        time = perform(_region_size, stride, 0);

    } while ( fabs(last-time) / time >= HyriseCalibratorConstants::eps &&
              stride <= _region_size / 2
        );

    _max_stride = stride;
    return stride;
}

/*
 * Prints the result
 */
void HyriseCalibrator::printResult()
{

    if (_results.size() == 0)
        return;
    
    size_t s = _results[0].x.size();
    size_t ser = _results.size();
   
    for (size_t i=0; i < s; ++i)
    {
        std::cout << i << " " << _results[0].x[i] << " ";
        
        for (size_t j=0; j < ser; ++j)
        {
            std::cout << _results[j].y[i] << " ";
        }
        std::cout << std::endl;
    }
}

void HyriseCalibrator::calculateBandwidth()
{
    std::cerr << "========================================" << std::endl;
    std::cerr << "Calculating Single Sequential Bandwidth" << std::endl;

    
    unsigned * data;
    int t =posix_memalign((void**) &data, 4096, _region_size);
    if (t != 0)
    {
        std::cout << "Memory Allocation Failed" << std::endl;
        exit(t);
    }
    
    ulng i = _region_size / sizeof(unsigned);
    long_long time = 0;
    __dummy = 0;

    #ifdef USE_SSE
    __m128i * src, res;
    #else
    ulng res;
    #endif

    
    for(ulng j=0; j < i; ++j)
        data[j] = j;

    // Adjust the size
    i /= 4;

    #ifdef USE_SSE
    // Cast the data to the integral type for SIMD
    src = (__m128i*) data;
    #endif
    
    {
        tracer trc(&time, _speed);
        while (i-- > 0)
        {
            #ifdef USE_SSE
            res = _mm_add_epi32(res, *(src + i));
            #else
            res = data[i*4] + data[i*4-1] + data[i*4-2] + data[i*4-3];
            #endif
        }
    }
    

    #ifdef USE_SSE
    __dummy += _mm_extract_epi32(res, 0);
    __dummy += _mm_extract_epi32(res, 1);
    __dummy += _mm_extract_epi32(res, 2);
    __dummy += _mm_extract_epi32(res, 3);
    #else
    __dummy = res;
    #endif

    ulng result = 1 / (time / (double) _speed) * _region_size;

    stats.single_sequential_bandwidth = result / 1024.0 / 1024.0;
}


struct t_info
{
    size_t s;
    size_t e;
    ulng  result;
    unsigned * data;
    long_long time;
    ulng speed;

    inline void print()
    {
        std::cout << "S:" << s << " E:" << e << " D:" << std::hex << data << std::dec << std::endl;
    }
    
};
    
void* sum_part(void * i)
{
    t_info * info = (t_info*) i;

    size_t e = info->s + info->e;
    ulng r = 0;
    
    for(size_t i=info->s; i < e; ++i)
        r+= *(info->data + i);

    info->result = r;
}


void HyriseCalibrator::calculateParallelBandwidth()
{
    std::cerr << "========================================" << std::endl;
    std::cerr << "Calculating Parallel Sequential Bandwidth" << std::endl;
    
    ulng num_elements = _region_size / sizeof(unsigned);
    ulng checksum = ((num_elements-1) * ((num_elements-1) + 1))/2;

    unsigned * data;
    int t = posix_memalign((void**) &data, 4096,  _region_size);
    if (t != 0)
    {
        std::cout << "Memory Allocation Failed" << std::endl;
        exit(t);
    }

    
    for(size_t i=0; i < num_elements; ++i)
        data[i] = i;
     
    
    size_t initial_cpu = 1;
    size_t max_cpu = HyriseCalibratorConstants::max_cpu;
    long_long time = 0;

    vector<unsigned long> result_times;

    for (size_t cpus = initial_cpu; cpus <= max_cpu; cpus+=2)
    {
        time = 0;
        pthread_t * threads = (pthread_t*) malloc(cpus * sizeof(pthread_t));
        t_info * infos = (t_info*) malloc(cpus * sizeof(t_info));
        
        size_t part_size = num_elements / cpus;
        size_t parts = num_elements / part_size;

        // Fill infos
        for(size_t i=0; i < cpus; ++i)
        {
            infos[i].s = i * part_size;
            infos[i].e = part_size ;
            infos[i].data = data;
            infos[i].speed = _speed;
            infos[i].result = 0;
        }

        {
            tracer trc(&time, _speed);
            for(size_t i=0; i < cpus; ++i)
            {
                pthread_create(&threads[i], NULL, sum_part, (void*) &infos[i]);
            }

            for(size_t i=0; i < cpus; ++i)
            {
                pthread_join( threads[i], NULL);
            }
        }

	result_times.push_back(time);

        free(threads);
        free(infos);

    }

    stats.max_sequential_bandwidth = 1;
    float max_bw = 1.0, new_bw;
    for(size_t i=1; i < result_times.size(); ++i)
    {
        if (result_times[i] > 0)
        {
            new_bw = result_times[0] / (double) result_times[i];
            if (new_bw > max_bw)
                max_bw = new_bw;
            
        }
    }

    if (stats.single_sequential_bandwidth)
        stats.max_sequential_bandwidth = max_bw * stats.single_sequential_bandwidth;
 
}


void * fib_call(void * param)
{
    t_info * info = (t_info*) param;
    
    ulng u = 0;
    ulng v = 1;
    ulng i, t, n;

    n = info->s;
    
    for(i = 2; i <= n; i++)
    {
        t = u + v;
        u = v;
        v = t;
    }

    info->result = v;
}

void HyriseCalibrator::calculateNumberOfCores()
{
    std::cerr << "========================================" << std::endl;
    std::cerr << "Calculating number of cores based on CPU bound operations" << std::endl;
    
    size_t initial_cpu = 1;
    size_t running, min_run = 3;
    size_t max_cpu =  HyriseCalibratorConstants::max_cpu;
    size_t max_fib = HyriseCalibratorConstants::loads *10000;

    vector<unsigned long> result_times;
    long_long time = 0;

    stats.num_cpus = 0;

    double best = 0, last, before_last = 0;
    size_t best_cpu = 0;
    
    for (size_t cpus = initial_cpu; cpus <= max_cpu; cpus+=1)
    {
        time = 0;
        vector<pthread_t> threads(cpus, pthread_t());
        
        vector<t_info> infos(cpus, t_info());
        

        for(size_t i=0; i < cpus; ++i)
        {
            infos[i].s = max_fib;
            infos[i].speed = _speed;
            infos[i].result = 0;
        }

        {
            tracer trc(&time, _speed);
            for(size_t i=0; i < cpus; ++i)
            {
                pthread_create(&threads[i], NULL, fib_call, (void*) &infos[i]);
            }

            for(size_t i=0; i < cpus; ++i)
            {
                pthread_join( threads[i], NULL);
            }
        }


        result_times.push_back(time);
        last = (double) (result_times[0]) / time * cpus;
        
        if (last > best)
        {
  	    best_cpu = cpus;
            best = last;
        }

        DEBUG(fabs(before_last - last) / last);
        if (fabs(before_last - last) / last < 0.01)
        {
            if (running >= min_run)
                break;
            else
                ++running;
        } else {
            running == 0;
        }

        before_last = last;
    }

    stats.num_cpus = best_cpu;

    if (debug)
    {
        std::cerr << "Iteration where throughput was max: " <<  best_cpu << std::endl;
        std::cerr << "Maximum Throughput overall: " << best << std::endl;
    }
        
    if (fabs(best - best_cpu) / best_cpu > HyriseCalibratorConstants::eps)
    {
        stats.has_hyperthreading = true;
        stats.num_cpus = floor(best);
    }
    else
    {
        stats.has_hyperthreading = false;
        stats.num_cpus = best_cpu;
    }
            
    
    result_times.clear();
}

/*
 *
 *  Since the cache results are random misses we have the same hops
 *  for all cache levels (at least most of them). Therefore we can
 *  start the analysis with the larges stride and find a pattrn when
 *  we have a plateau.
 */
HyriseCalibrator::cache_hierarchy_t HyriseCalibrator::analyzeCacheRun()
{
    size_t min_keep_length = 3;
    size_t act_keep_length = 0;

    size_t max_run = _results[0].x.size();
    size_t max_horizontal = _results.size();

    size_t hor_keep;
    
    bool new_level = false, has_diff;

    // Hierarchy
    cache_hierarchy_t hierarchy;
    HyriseCalibratorCache c;


    long last, current, cache_line_draft;
    double diff;
    
    for(size_t i = 2; i < max_run; ++i)
    {

        if (_results[0].x[i-1] == 0)
            continue;
        
        has_diff = false;
        hor_keep = 0;

        // Find all where the diff is > eps_cache
        for (size_t x = 0; x < max_horizontal; ++x)
        {
            last = _results[x].y[i-1];
            current = _results[x].y[i];

            diff = fabs(last - current) / (double) last;

            has_diff = diff > HyriseCalibratorConstants::eps_cache;
            if (has_diff)
            {
                cache_line_draft = (max_horizontal - x);
                ++hor_keep;
            }
        }

        if (hor_keep >= min_keep_length)
        {

            new_level = true;
            c = HyriseCalibratorCache();
            c.size = _results[0].x[i-1];
            c.cache_line = test_s * pow(2,cache_line_draft);

            // Reset the keep length
            act_keep_length = 0;
            
        } else {

            ++act_keep_length;

            if (act_keep_length >= min_keep_length && new_level)
            {
                hierarchy.push_back(c);
                new_level = false;
            }
        }
    }

    return hierarchy;
}

template<typename T>
void HyriseCalibrator::fillCLTest(T * vals, size_t draft)
{
    size_t actual = draft / sizeof(T);
    size_t pad = sizeof(T)/WRD_SIZE - PAD_BASE/WRD_SIZE;

    std::vector<size_t> list;
    size_t last;

    for (size_t i=0; i < actual; ++i)
        list.push_back(i);

    std::random_shuffle(list.begin(), list.end());
    
    for (size_t i=1; i < list.size(); ++i)
    {
        vals[list[i-1]].n = &vals[list[i]];

        for(size_t j=0; j < pad; ++j)
            vals[list[i-1]].v[j] = j;

        vals[list[i]].n = NULL;
    }

    for(size_t j=0; j < pad; ++j)
        vals[list.back()].v[j] = j;

    // Circle
    vals[list.back()].n = &vals[list[0]];
}


#define EXE(o) t = t->n; su += t->v[o];
#define EFIVE(o) EXE(o) EXE(o) EXE(o) EXE(o) EXE(o)
#define ETEN(o) EFIVE(o) EFIVE(o)
#define EHUND(o) ETEN(o) ETEN(o) ETEN(o) ETEN(o) ETEN(o) ETEN(o) ETEN(o) ETEN(o) ETEN(o) ETEN(o)


size_t HyriseCalibrator::getCacheLineSize(size_t draft)
{
    DEBUG("------------------------------ DRAFT " << draft);
    
    cl_test_t_512 * vals;

    long reduce;

    if (draft < 1024 * 128)
        reduce = 10000;
    else if (draft < 1024*1024)
        reduce = 1000;
    else
        reduce = 10;
    

    // Allocate Memory
    NumaHandler<SimpleMemoryTrait> handler;
    vals = (cl_test_t_512*) handler.allocateWithNode(0, draft);

    long_long tmp=0, last=0;
    size_t upper_bound = sizeof(cl_test_t_512)/WRD_SIZE - PAD_BASE/WRD_SIZE;
    
    for(size_t i=0; i < upper_bound ;++i)
    {
        DEBUG("Padding " << i * WRD_SIZE + sizeof(void*) );
        tmp = 0;
        for(size_t j=0; j < HyriseCalibratorConstants::repeat; ++j)
        {
            register long su = 0;

            // Fill the data
            fillCLTest<cl_test_t_512>(vals, draft);
            
            long inc = HyriseCalibratorConstants::loads * reduce;
            cl_test_t_512 * t = vals;
            
            {
                tracer trc(&tmp, _speed);
                while (inc > 0)
                {
                    EHUND(i);
                    inc -= 100;
                }
            }

            __dummy += (long) t;
            __dummy += su;
            
            //DEBUG("Time " << tmp);
            tmp += tmp;
        }

        
        tmp /= (double) HyriseCalibratorConstants::repeat;
        DEBUG("Diff " << tmp << " " << last << " " << fabs(tmp-last)/(double)tmp);
        
        if (i > 1 && fabs(tmp-last)/(double)tmp > 0.05)
        {
            return i * WRD_SIZE + sizeof(void*);
        }
        
        last = tmp;
    }


    // Free Mem
    handler.freeMemory(vals, _region_size);
    return 0;
}


HyriseCalibrator::cache_hierarchy_t HyriseCalibrator::analyzeCache()
{
    std::cerr << "========================================" << std::endl;
    std::cerr << "Analyzing Cache Hierarchy" << std::endl;
    
    determineMaxStride();
    cache_hierarchy_t finals, randoms;

    
    // First perform a random traversal to
    // get a good look at the cache sizes
    combinedTraversal(true, 0);

    if (debug)
        printResult();

    //randoms = analyzeCacheRun();

    // Now check for the cache line size using sequential
    series_t random_results = _results;


    // Detect cache line for each cache level
    eraseData();

    combinedTraversal(false, 0);
    finals = analyzeCacheRun();

    for (size_t i=0; i < finals.size(); ++i)
    {
        std::cout << fixed_cl_size << std::endl;
        finals[i].cache_line = HyriseCalibrator::fixed_cl_size > 0 ? HyriseCalibrator::fixed_cl_size : getCacheLineSize(finals[i].size);
    }

    
    if (debug)
        printResult();


    // Now add the latencies
    for (size_t i=0; i < finals.size(); ++i)
    {
        // if cl size is 64 this must be a multiple of test_s*2^x
        //8 * 2^x = 64 =>   8 = 2^x
        finals[i].latency = extractLatency(finals[i].size, _results)[log2(finals[i].cache_line / 8)];
        finals[i].random_latency = extractLatency(finals[i].size, random_results)[log2(finals[i].cache_line/ 8)];
    }


    
    return finals;
}

vector<ulng> HyriseCalibrator::extractLatency(ulng range, series_t data)
{
    vector<ulng> result;
    bool finish=false;
    for (size_t i=0; i < data[0].x.size(); ++i)
    {
        if (data[0].x[i] == range)
        {
            finish = true;
            continue;
        }

        if (finish)
        {
            for (size_t j=0; j < data.size(); ++j)
            {
                result.push_back(data[j].y[i]);
            }
            break;
        }

    }
    return result;
}

unsigned HyriseCalibrator::matchLatency(vector<ulng> lat)
{
    assert(lat.size() > 0);
    long last = lat[0], current;
    double diff;
    
    for(size_t i=0; i < lat.size(); ++i)
    {
        current = lat[i];
        diff = fabs(current - last) / (double) last;

        if (diff > HyriseCalibratorConstants::eps_cache)
            return i-1;
    }
}

unsigned HyriseCalibrator::avgLatency(vector<ulng> lat)
{
    double diff =0;
    for (size_t i=0; i < lat.size(); ++i)
        diff += lat[i];

    return round(diff / lat.size());
}

void HyriseCalibrator::runAll()
{
    if (config.do_cpu)
        calculateNumberOfCores();

    if (config.do_bandwidth)
    {
        calculateBandwidth();
        calculateParallelBandwidth();
    }

    if (config.do_cache)
        stats.caches = analyzeCache();

    if (config.do_numa)
        calculateNUMAOverhead();
}

void HyriseCalibrator::calculateNUMAOverhead()
{



    std::cerr << "========================================" << std::endl;
    std::cerr << "NUMA Analysis" << std::endl;
    
    // Before else, free what we have allocated 
    freeMemory();

#ifdef USE_NUMA
    NumaHandler<NumaMemoryTrait> handler;
#else
    NumaHandler<SimpleMemoryTrait> handler;
#endif
    NumaOptions options;
    options.restrictExecutionToNode = true;

    stats.num_nodes = handler.getNumNodes();
    
    for(int i=0; i < handler.getNumNodes(); ++i)
    {
        // Allocate Memory
        __data = (test_t*) handler.allocateWithNode(i, _region_size, options);

        // We only want to performa a combined traversal
        // with random and seq reading
        {
            HyriseCalibratorCache l = stats.caches.back();
            series_t tmp;
            tmp.push_back(traversal(l.cache_line, true, 0));

            vector<ulng> res = extractLatency(l.size, tmp);

            // Add the miss to the list
            stats.numa_misses_rand.push_back(res[0]);

            tmp.clear();
            tmp.push_back(traversal(l.cache_line, false, 0));

            res = extractLatency(l.size, tmp);

            // Add the miss to the list
            stats.numa_misses_seq.push_back(res[0]);


            
            
        }

        // Free Memory
        handler.freeMemory((void*)__data, _region_size);
    }
    __data = NULL;
}



// Default initializer with false
bool HyriseCalibrator::debug = false;
unsigned HyriseCalibrator::fixed_cl_size = 0;


int main(int argc, char** argv)
{
    __dummy = 0;
    
    if (argc < 3)
    {
        std::cout << "Usage: hycal [speed in Mhz] [range in MB] [do_cpu=1] [do_bandwith=1] [do_cache=1] [do_numa=1]" <<std::endl;
        exit(1);
    }


    ulng base = 1024 * 1024;
    ulng other = (ulng) atol(argv[2]);
    size_t s = base * other;
    unsigned long speed = (ulng) atol(argv[1]) * 1000000;

    ////////////////////////////////////////////////////////////////////
    /// This is where the test starts
    HyriseCalibrator c(s, speed);

    #ifndef NDEBUG
    HyriseCalibrator::debug = true;
    #endif

    // Hack to set the size
    HyriseCalibrator::fixed_cl_size = 64;

    
    switch (argc)
    {
    case 7:
        c.config.do_numa = atoi(argv[6]) == 1;
        // FALL THROUGH
    case 6:
        c.config.do_cache = atoi(argv[5]) == 1;
        // FALL THROUGH
    case 5:
        c.config.do_bandwidth = atoi(argv[4]) == 1;
        // FALL THROUGH
    case 4:
        c.config.do_cpu = atoi(argv[3]) == 1;
        break;
    }

    c.runAll();
    c.stats.print();
    
    return return_codes::OK;
}
