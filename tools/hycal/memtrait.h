#ifndef HYCAL_MEMTRAIT_H
#define HYCAL_MEMTRAIT_H

#define PAGE_SIZE 4096

#ifdef USE_NUMA
#include <numa.h>
#endif

#include <iostream>

#define ERR_EXIT(msg) do { std::cout << msg << std::endl; exit(1); } while(false)

struct NumaOptions
{
    NumaOptions(): restrictExecutionToNode(false), executionOnNode(0)
    {}

    bool restrictExecutionToNode;
    unsigned int executionOnNode;
    
};


#ifdef USE_NUMA
struct NumaMemoryTrait
{
    static int getNumNodes()
    {
        return numa_num_configured_nodes();
    }

    static void * allocate(int node, size_t size, NumaOptions options)
    {
        void * result;
        result = numa_alloc_onnode(size, node);
        if (result == NULL)
            ERR_EXIT("Could not allocate memory using numa_alloc_onnode");

        if (options.restrictExecutionToNode)
        {
            struct bitmask * mask = numa_allocate_nodemask();
            numa_bitmask_clearall(mask);

            if (options.executionOnNode >= getNumNodes())
                ERR_EXIT("Cannot execute on a higher node than available");
            
            numa_bitmask_setbit(mask, options.executionOnNode);
            numa_run_on_node_mask(mask);
        }
        return result;
    }

    static void freeMemory(void * mem, size_t size)
    {
        numa_free(mem, size);
        mem = NULL;
        // Allow the kernel free scheduling again
        numa_run_on_node_mask(numa_all_nodes_ptr);
    }
};
#endif


struct SimpleMemoryTrait
{
    static int getNumNodes() { return 1; }

    static void * allocate(int node, size_t size, NumaOptions options)
    {
        void * result;
        int t = posix_memalign(&result, PAGE_SIZE, size);
        if (t != 0)
        {
            exit(t);
        }
        return result;
    }

    static void freeMemory(void * mem, size_t size)
    {
        free(mem);
        mem = NULL;
    }
};


template<typename T>
struct NumaHandler
{


    NumaHandler(){};

    /*
     * Allocate a memory region on a certain numa node with options
     */
    void * allocateWithNode(int node,
                            size_t size,
                            NumaOptions options = NumaOptions())
    {
        return T::allocate(node, size, options);
    }


    void freeMemory(void * m, size_t size)
    {
        T::freeMemory(m, size);
    }
    
    
    int getNumNodes()
    {
        return T::getNumNodes();
    }
    
};

#endif // HYCAL_MEMTRAIT_H

