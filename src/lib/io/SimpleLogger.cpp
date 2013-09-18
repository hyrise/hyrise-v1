#include "io/SimpleLogger.h"
#include <errno.h>
#include <string.h>

namespace hyrise {
namespace io {

SimpleLogger &SimpleLogger::getInstance() {
    static SimpleLogger instance;
    return instance;
}

void SimpleLogger::logValue(const tx::transaction_id_t transaction_id,
                            const storage::table_id_t table_id,
                            const storage::pos_t row,
                            const storage::pos_t invalidated_row,
                            const uint64_t field_bitmask,
                            const ValueIdList *value_ids) {
    std::stringstream ss;
    ss << "(v," << (int)transaction_id << "," << (int)table_id << "," << (int)row << "," << (int)invalidated_row << "," << field_bitmask << ",(";
    if(value_ids != nullptr) {
        ss << (*value_ids)[0].valueId;
        for(auto it = ++value_ids->cbegin(); it != value_ids->cend(); it++)
            ss << "," << it->valueId;
    }
    ss << "))";
    _mutex.lock();
    write(_fd, (void *) ss.str().c_str(), ss.str().length());
    fsync(_fd);
    _mutex.unlock();
}

void SimpleLogger::logCommit(const tx::transaction_id_t transaction_id) {
    std::stringstream ss;
    ss << "(t," << transaction_id << ")";
    _mutex.lock();
    write(_fd, (void *) ss.str().c_str(), ss.str().length());
    fsync(_fd);
    _mutex.unlock();
}

SimpleLogger::SimpleLogger() {
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    _fd = open("logfile", O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (_fd == -1)
        printf( "Something went wrong while creating the logfile: %s\n", strerror( errno ) );
}

}
}
