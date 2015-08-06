#pragma once

#include <atomic>
#include <cstdio>
#include <cstring>
#include <mutex>

#include "storage/storage_types.h"
#include "helper/types.h"
#include <helper/barrier.h>

namespace hyrise {
namespace io {

class BufferedLogger {
 public:
  BufferedLogger(const BufferedLogger&) = delete;
  BufferedLogger& operator=(const BufferedLogger&) = delete;

  static BufferedLogger& getInstance();

  template <typename T>
  inline T read_value(char*& cursor) {
    cursor -= (sizeof(T) - 1);
    auto value = *(T*)cursor;
    --cursor;
    return value;
  }

  template <typename T>
  inline std::string read_string(char*& cursor) {
    auto size = read_value<T>(cursor);
    cursor -= (size - 1);
    std::string text(cursor, size);
    --cursor;
    return std::move(text);
  }

  template <typename T>
  inline void write_value(char*& cursor, const T value) {
    *(T*)cursor = value;
    cursor += sizeof(T);
  }

  template <typename T>
  inline void write_string(char*& cursor, const std::string& value) {
    memcpy(cursor, value.c_str(), value.size());
    cursor += value.size();
    *(T*)cursor = (T)value.size();
    cursor += sizeof(T);
  }

  template <typename T>
  inline void logDictionary(const std::string& table_name,
                            storage::field_t column,
                            const T& value,
                            storage::value_id_t value_id) {
    if (table_name.empty())
      return;
    char entry[90];
    char* cursor = entry;
    logDictionaryValue(cursor, value);
    write_value<storage::value_id_t>(cursor, value_id);
    write_value<storage::field_t>(cursor, column);
    write_string<char>(cursor, table_name);
    write_value<char>(cursor, 'D');
    unsigned int len = cursor - entry;
    assert(len <= 90);
    _append(entry, len);
  }

  // used for all data types where their size is sizeof(T)
  template <typename T>
  inline void logDictionaryValue(char*& cursor, const T& value) {
    write_value<T>(cursor, value);
    write_value<int>(cursor, sizeof(value));
  }

  void logValue(const tx::transaction_id_t transaction_id,
                const std::string& table_name,
                const storage::pos_t row,
                const ValueIdList* value_ids);

  void logInvalidation(const tx::transaction_id_t transaction_id,
                       const std::string& table_name,
                       const storage::pos_t invalidated_row);

  void logCommit(tx::transaction_id_t transaction_id);
  void logRollback(tx::transaction_id_t transaction_id);
  void flush(bool blocking = true);
  void truncate();
  void restore(const size_t thread_count);

  size_t startCheckpoint();
  size_t endCheckpoint();
  void logStartCheckpoint(const size_t checkpoint_id);
  void logEndCheckpoint(const size_t checkpoint_id);
  void openNextLogfile();

  size_t readLastCheckpointID();
  void writeLastCheckpointID(size_t checkpoint_id);
  std::string getLogfilenameForCheckpoint(size_t checkpoint_id);

  size_t getLastCheckpointID() {
    return _checkpoint_id;
  };

 private:
  BufferedLogger();

  void _append(const char* str, const unsigned char len);

  void restore_thread(char* logfile,
                      size_t start_block,
                      size_t end_block,
                      size_t leftovers,
                      size_t thread_id,
                      std::vector<bool>& committed_tid_bitvector,
                      std::vector<bool>& rolledback_tid_bitvector,
                      thread_barrier& barrier);

  char* getBufferWriteArea(const size_t size);
  void writePaddingEntry(size_t absolute_write_pos, size_t padding);

  inline char* getBufferPointerAtPos(size_t pos) { return _buffer + (pos % _buffer_capacity); }

  inline unsigned char getBufferValueAtPos(size_t pos) { return *((unsigned char*)getBufferPointerAtPos(pos)); }

  FILE* _logfile;
  char* _buffer;
  char* _head;
  char* _tail;

  size_t _last_flush_pos;
  size_t _buffer_capacity;

  std::mutex _checkpointMutex;
  std::mutex _fileMutex;
  std::mutex _flushMutex;
  std::atomic<size_t> _buffer_size;
  std::atomic<size_t> _total_logsize;
  std::atomic<size_t> _checkpoint_id;

  bool _changes_since_last_checkpoint;
  std::string _logdir;
};

template <>
void BufferedLogger::logDictionaryValue(char*& cursor, const storage::hyrise_string_t& value);
}
}
