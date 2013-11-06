#include "fs.h"

#include <unistd.h>
#include <stdexcept>
#include <memory>

namespace hyrise {namespace helper {

std::string sys_getcwd()
{
    const size_t chunkSize=255;
    const int maxChunks=10240; // 2550 KiBs of current path are more than enough

    char stackBuffer[chunkSize]; // Stack buffer for the "normal" case
    if(getcwd(stackBuffer,sizeof(stackBuffer))!=nullptr)
        return stackBuffer;
    if(errno!=ERANGE)
    {
        // It's not ERANGE, so we don't know how to handle it
        throw std::runtime_error("Cannot determine the current path.");
        // Of course you may choose a different error reporting method
    }
    // Ok, the stack buffer isn't long enough; fallback to heap allocation
    for(int chunks=2; chunks<maxChunks ; chunks++)
    {
        // With boost use scoped_ptr; in C++0x, use unique_ptr
        // If you want to be less C++ but more efficient you may want to use realloc
        std::unique_ptr<char> cwd(new char[chunkSize*chunks]); 
        if(getcwd(cwd.get(),chunkSize*chunks)!=nullptr)
            return cwd.get();
        if(errno!=ERANGE)
        {
            // It's not ERANGE, so we don't know how to handle it
            throw std::runtime_error("Cannot determine the current path.");
            // Of course you may choose a different error reporting method
        }   
    }
    throw std::runtime_error("Cannot determine the current path; the path is apparently unreasonably long");
}
	
}}