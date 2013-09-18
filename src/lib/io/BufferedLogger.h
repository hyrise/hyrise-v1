#ifndef SRC_LIB_IO_BUFFEREDLOGGER_H
#define SRC_LIB_IO_BUFFEREDLOGGER_H

#include <atomic>
#include <cstdio>
#include <cstring>
#include <mutex>

#include "storage/storage_types.h"
#include "helper/types.h"

namespace hyrise {
namespace io {

class BufferedLogger {
public:
    BufferedLogger(const BufferedLogger&) = delete;
    BufferedLogger &operator=(const BufferedLogger&) = delete;

    static BufferedLogger &getInstance();

    template<typename T>
    void logDictionary(storage::table_id_t table_id,
                       storage::field_t column,
                       const T &value,
                       storage::value_id_t value_id);
    void logValue(tx::transaction_id_t transaction_id,
                  storage::table_id_t table_id,
                  storage::pos_t row,
                  storage::pos_t invalidated_row,
                  uint64_t field_bitmask,
                  const ValueIdList *value_ids);
    void logCommit(tx::transaction_id_t transaction_id);

private:
    BufferedLogger();

    void _append(const char *str, const unsigned int len);
    void _flush();

    FILE *_logfile;
    char *_buffer;
    char *_head;
    char *_tail;
    char *_last_write;
    uint64_t _buffer_size;

    std::mutex _bufferMutex;
    std::mutex _fileMutex;
    std::atomic<int> _writing;
    std::atomic<uint64_t> _size;
};

}
}

#endif // SRC_LIB_IO_BUFFEREDLOGGER_H
