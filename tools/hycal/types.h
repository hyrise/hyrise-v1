#ifndef HYCAL_TYPES_H
#define HYCAL_TYPES_H

#include <vector>
#include <iostream>
#include <sstream>

typedef unsigned long ulng;


#define NPAD 0
#define PAD_BASE sizeof(void*)
#define WRD_SIZE sizeof(unsigned)
#define CL_TEST_TYPE(size) typedef struct _cl_test_t_##size { _cl_test_t_##size * n; unsigned v[size/WRD_SIZE - PAD_BASE/WRD_SIZE]; } cl_test_t_##size;

CL_TEST_TYPE(512)
CL_TEST_TYPE(256)
CL_TEST_TYPE(128)
CL_TEST_TYPE(64)

using namespace std;


/*
 * Defines general used constants for the calibrator
 */
struct HyriseCalibratorConstants
{
    const static double eps = 0.1;
    const static double eps_cache = 0.3;
    const static unsigned loads = 100000;
    const static unsigned mintime = 100000;
    const static unsigned repeat = 3;
    const static size_t max_cpu = 128;
};

/*
 * Return codes of the application
 */
struct return_codes
{
    enum {
        OK,
        ERROR
    } return_code_t;
};


/*
 * Struct to define the properties of a cache in the memory hierarchy
 *
 * identifies the size, the random and sequential latency plus the
 * size of the cache line
 */
struct HyriseCalibratorCache
{
    size_t size;
    size_t latency;
    size_t random_latency;
    size_t cache_line;

    HyriseCalibratorCache():size(0), latency(0), random_latency(0), cache_line(0)
    {}
    
    std::string formatSize()
    {
        std::stringstream str;
        size_t n = size >= 1024*1024 ? (size / 1024 / 1024) : (size / 1024);

        str << n << ((size >= 1024*1024) ? " MB " : " kB ");
        return str.str();
    }

    std::string formatCacheLine()
    {
        std::stringstream str;
        str << cache_line << " b ";
        return str.str();
    }

    std::string formatLatency()
    {
        std::stringstream str;
        str << latency << " cyc";
        return str.str();
    }
    
    std::string format()
    {
        std::stringstream str;
        str.width(30); str.fill(' ');
        str << std::left << "Size: ";
        str.width(10); str.fill(' ');
        str << std::right << formatSize();

        str.width(30); str.fill(' ');
        str << std::left << "Cache Line Size: ";
        str.width(10); str.fill(' ');
        str << std::right << formatCacheLine() << std::endl;

        str.width(25); str.fill(' ');
        str << std::left << "Seq Latency: ";
        str.width(10); str.fill(' ');
        str << std::right << latency << " cyc ";

        str.width(25); str.fill(' ');
        str << std::left << "Rand Latency: ";
        str.width(10); str.fill(' ');
        str << std::right << random_latency << " cyc " << std::endl;


        return str.str();
    }
    
    void print()
    {
        std::cout << format();
    }
};


/*
 * This is the main statistics module. In this struct all essential
 * informaiton captured by the calibrator is stored.
 *
 */
struct HyriseCalibratorStatistics
{
    // Caches
    vector<HyriseCalibratorCache> caches;

    // Bandwidth
    ulng single_sequential_bandwidth;
    ulng max_sequential_bandwidth;
    
    // Cores
    size_t num_cpus;
    bool has_hyperthreading;
    
    // NUMA
    size_t num_nodes;
    vector<ulng> numa_misses_rand;
    vector<ulng> numa_misses_seq;


    HyriseCalibratorStatistics():
        single_sequential_bandwidth(0), max_sequential_bandwidth(0), num_cpus(0),
        has_hyperthreading(false), num_nodes(0)
    {}

    
    void print()
    {
        std::stringstream str;
        str << std::endl << std::endl;
        str.width(80);
        str.fill(' ');
        str << std::left <<  "Hyrise Calibrator (c) Martin Grund 2010" << std::endl;
        str.fill('=');
        str.width(80);
        str << std::left << "=" << std::endl;

        // Bandwidth
        str.width(25); str.fill(' ');
        str << std::left << "Sequential Bandwidth:";
        str.width(9); str.fill(' ');
        str << std::right << single_sequential_bandwidth << " MB/s ";

        str.width(25); str.fill(' ');
        str << std::left << "Parallel Bandwidth:";
        str.width(9); str.fill(' ');
        str << std::right << max_sequential_bandwidth << " MB/s " << std::endl << std::endl;

        // Cores
        str.width(30); str.fill(' ');
        str << std::left << "Number of Cores:";
        str.width(9); str.fill(' ');
        str << std::right << num_cpus << " ";
        str.width(30); str.fill(' ');
        str << std::left << "Hyperthreading:";
        str.width(9); str.fill(' ');
        str << std::right << std::boolalpha << has_hyperthreading << std::endl;
        str << std::endl;

        str.fill('-');
        str.width(80);
        str << std::left << "-" << std::endl;
        
        str << "CPU Caches" << std::endl;

        for(size_t i=0; i < caches.size(); ++ i)
        {
            str << "Level " << (i+1) << std::endl;
            str << caches[i].format() << std::endl;
        }

        str << std::endl;

        str.fill('-');
        str.width(80);
        str << std::left << "-" << std::endl;


        
        str << "NUMA Overhead";
        str.fill(' ');
        str.width(27);
        str << std::right << "Random Access ";
        str.fill(' ');
        str.width(39);
        str << std::right <<  "Sequential Access" << std::endl;

        
        for(size_t i=0; i < num_nodes; ++i)
        {
            str << "Node " << i;
            
            
            str.fill(' ');
            str.width(28);
            str << std::right <<  numa_misses_rand[i] << " cyc ";
            str.fill(' ');
            str.width(36);
            str << std::right <<  numa_misses_seq[i] << " cyc" << std::endl;
        }
        
        str.fill('=');
        str.width(80);
        str << std::left << "=" << std::endl;

        
        std::cout << str.str() << std::endl;
    }
    
};


#endif // HYCAL_TYPES_H
