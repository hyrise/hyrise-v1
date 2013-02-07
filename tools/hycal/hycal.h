#ifndef HYRISE_CALIBRATOR_H
#define HYRISE_CALIBRATOR_H

#include "trace.h"
#include "memtrait.h"
#include "types.h"


#include <stdlib.h>
#include <assert.h>
#include <vector>
#include <algorithm>
#include <iostream>

#include <fstream>


unsigned long __dummy;


/*
  
 */
class HyriseCalibrator
{

public:

    // typedef
    typedef unsigned long ulng;
    typedef vector<HyriseCalibratorCache> cache_hierarchy_t;

    typedef struct _config_t
    {
        bool do_cpu;
        bool do_cache;
        bool do_bandwidth;
        bool do_numa;

        _config_t(): do_cpu(true), do_cache(true), do_bandwidth(true), do_numa(true)
        {}
        
    } config_t;

private:

    ofstream fp;
    
    typedef struct {
        vector<ulng> x;
        vector<ulng> y;
        ulng stride;
    } intermediate;

    // Follow Element
    typedef struct _test_t {
        struct _test_t *n;
        long int pad[NPAD];
    } test_t;

    
    // Type for capturing the series
    typedef vector<intermediate> series_t;


    // Data is a list of chained elements
    test_t* __data;

    // Region Size
    size_t _region_size;

    // Processor Speed in Hz
    ulng _speed;

    // Data
    series_t _results;

    // sizeof(test_t)
    size_t test_s;

    // Maximum stride to use
    ulng _max_stride;

    
    vector<ulng> extractLatency(ulng range, series_t data);

    unsigned matchLatency(vector<ulng> lat);

    unsigned avgLatency(vector<ulng> lat);

    template<typename T>
    void fillCLTest(T * d, size_t draft);

    void allocateMemory();

    void freeMemory();
    
public:

    static bool debug;

    static unsigned fixed_cl_size;
    
    // Statistics Result
    HyriseCalibratorStatistics stats;
    

    // Set the configuration
    config_t config;
    
    
    /* Preparing the calibrator
     *
     * @param size_t region size of the region in bytes
     */
    HyriseCalibrator(size_t region, ulng speed);
    
    ~HyriseCalibrator();

    // Test methods follow
    intermediate traversal(ulng stride=1, bool random=false, int delay=0);

    //
    void combinedTraversal(bool random=false, int delay=0);

    /*
     * Performs the analysis of the cache data based on the last run
     *
     * Based on the previous results this method analyzes the captured
     * data and tries to estimate the different paramters for the
     * caches.
     */
    cache_hierarchy_t analyzeCacheRun();


    /*
     * Main test method to anlayze cache sizes and latency
     *
     */
    cache_hierarchy_t analyzeCache();
    
    /*
     * Performs a traversal with a certain stride on a given range,
     * data must be prepared before
     *
     * @param range the max range to traverse
     *
     */
    ulng perform(ulng range, ulng stride, int delay);

    /*
     * Called initially to detect what the largest possible stride
     * between two elements is. The results of this methods are only
     * used to give an upper bound for possible cache line sizes.
     */
    ulng determineMaxStride();

    /*
     * Print the results collected by perfomr() / analyzeCacheRun()
     */
    void printResult();

    /*
     * Fill a region with values of type test_t by concatenating the
     * pointers sequentially
     *
     * @param s region size in bytes
     * @param stride the distance between two elements in bytes
     */
    void fillRegion(size_t s=0, size_t stride=1);

    /*
     * Fills a regions with randomly distributed values of type
     * test_t. The list can be traversed by following th next pointer
     * of each element.
     *
     * @param s the size of the region in bytes
     * @param stride the distance between two elements in bytes
     */
    void fillRegionRandom(size_t s=0, size_t stride=1);


    /*
     * Calculate the available Bandwidth
     *
     * Based on simple experiments try to calculate the available
     * bandwidth for memory bound operations.
     */
    void calculateBandwidth();


    /*
     * Caclulates the maximum available streaming bandwidth
     *
     * Using parallel threads this method tries to reach the maximum
     * available streaming bandwidth for all cores.
     *
     * @see calculateBandwidth()
     */
    void calculateParallelBandwidth();


    /*
     * Calulate the number of cores
     *
     * Using an iterative Fibonacci version this method tries to
     * allocate as much computing power as possible. In addition it
     * tries to estimate if the CPUs have some kind of hyperthreading.
     */
    void calculateNumberOfCores();


    /*
     * Using libnuma the cache efficiency using NUMA is determined.
     *
     * Based on the number of available nodes, memory is allocated and
     * accessed.
     */
    void calculateNUMAOverhead();


    /*
     * Analyzes the cache sizes and detects the cache line size per
     * level
     *
     * Based on the analysis of the size of the cache this method
     * detects the size of the cache line. This is done by randomly
     * traversing a region of the size of the cache. Now a second
     * access is added that accesses an element with a certain
     * distance to the first element. At the point where this second
     * access triggers a second cache miss a difference is measurable
     * and we can deduct the cache line size.
     */
    size_t getCacheLineSize(size_t draft);
    
    
    /*
     * Run all of the above tests
     */
    void runAll();
    
    void eraseData() {
        _results.clear();
    }


};


#endif
