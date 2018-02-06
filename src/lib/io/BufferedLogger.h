#pragma once

#include <atomic>
#include <cstdio>
#include <cstring>
#include <mutex>

#include "storage/storage_types.h"
#include "helper/types.h"
#include <helper/barrier.h>

#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_vector.h>

namespace hyrise {
namespace io {

class BufferedLogger {
 public:
  BufferedLogger(const BufferedLogger&) = delete;
  BufferedLogger& operator=(const BufferedLogger&) = delete;

  static BufferedLogger& getInstance();

  template <typename T>
  inline T read_value(const char*& cursor) {
    cursor -= (sizeof(T) - 1);
    auto value = *(T*)cursor;
    --cursor;
    return value;
  }

  template <typename T>
  inline std::string read_string(const char*& cursor) {
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
    char entry[530];
    char* cursor = entry;
    logDictionaryValue(cursor, value);
    write_value<storage::value_id_t>(cursor, value_id);
    write_value<storage::field_t>(cursor, column);
    write_string<char>(cursor, table_name);
    write_value<char>(cursor, 'D');
    unsigned int len = cursor - entry;
    assert(len <= 530);
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

  size_t logCommit(tx::transaction_id_t transaction_id);
  void logRollback(tx::transaction_id_t transaction_id);
  void flush(bool blocking = true, size_t up_to_pos = 0);
  void writeLogMeta(size_t pos_in_logfile, tx::transaction_id_t last_tid);
  void truncate();
  void restore(const size_t thread_count);
  void replicate(const char* logfile, size_t size, bool failover=false);

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

  bool checkpointFound() {
    return _checkpointFound;
  }

 private:
  BufferedLogger();

  size_t _append(const char* str, const size_t len);

  void restore_thread(char* logfile,
                      size_t start_block,
                      size_t end_block,
                      size_t leftovers,
                      size_t thread_id,
                      std::vector<bool>& committed_tid_bitvector,
                      std::vector<bool>& rolledback_tid_bitvector,
                      thread_barrier& barrier);
  void replicate_thread(const char* logfile,
                        size_t start_block,
                        size_t end_block,
                        size_t leftovers,
                        size_t thread_id,
                        tx::transaction_cid_t lastCID,
                        thread_barrier& barrier,
                        const char *&end_pos);

  size_t getBufferWriteArea(const size_t size);
  void writePaddingEntry(size_t absolute_write_pos, size_t padding);

  inline char* getBufferPointerAtPos(size_t pos) { return _buffer + (pos % _buffer_capacity); }

  template<typename T>
  inline T getBufferValueAtPos(size_t pos) { return *((T*)getBufferPointerAtPos(pos)); }

  FILE* _logfile;
  FILE* _logmetafile;
  char* _buffer;
  char* _head;
  char* _tail;

  size_t _last_flush_pos;
  size_t _buffer_capacity;

  std::mutex _checkpointMutex;
  std::mutex _fileMutex;
  std::mutex _flushMutex;
  std::mutex _resizeMutex;
  std::atomic<size_t> _buffer_size;
  std::atomic<size_t> _total_logsize;
  std::atomic<size_t> _checkpoint_id;

  bool _checkpointFound;
  bool _changes_since_last_checkpoint;
  std::string _logdir;

  tx::transaction_id_t _last_tid;

  // only for replication
  tx::transaction_id_t _lastCommittedTID;
  tbb::concurrent_unordered_map<tx::transaction_id_t, uintptr_t> _committed;
  tbb::concurrent_unordered_map<tx::transaction_id_t, bool> _rolled_back;
  tbb::concurrent_vector<std::tuple<storage::store_ptr_t, tx::transaction_id_t, storage::pos_t>> _pending_inserts;
  tbb::concurrent_vector<std::tuple<storage::store_ptr_t, tx::transaction_id_t, storage::pos_t>> _pending_deletes;
  bool _skipRedundant;
};

template <>
void BufferedLogger::logDictionaryValue(char*& cursor, const storage::hyrise_string_t& value);

template <>
inline void BufferedLogger::write_value(char*& cursor, const tx::transaction_id_t value) {
  *(tx::transaction_id_t*)cursor = value;
  cursor += sizeof(tx::transaction_id_t);
  _last_tid = value;
}

}
}
