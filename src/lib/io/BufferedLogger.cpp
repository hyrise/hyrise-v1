/*
 * Logging Format:
 *
 * The logfile is designed to be read backwards.
 * The logfile is aligned to a specified BLOCKSIZE by using skip entries.
 * It is guranteed that every offset that is a multiple of BLOCKSIZE points to the beginning of a log entry.
 * Entries are not separated. Instead, their length is implicitly or explicitly known.
 * After identifying the type of an entry, the next entry can be found by adding the size of the entry
 * Every entry has the structure Data + (char)Type
 *
 * The following entry types are distinguished:
 *
 * Possible Log Entries:
 *     Dictionary Entries:
 *       - value                : length(value)
 *       - length(value)        : sizeof(int)
 *       - value_id             : sizeof(value_id_t)
 *       - field_id             : sizeof(field_id_t)
 *       - table_name           : table_name.size()
 *       - table_name.size()    : sizeof(char)
 *       - type ("D")           : sizeof(char)
 *
 *     Value Entries:
 *       - list of value_ids    : store->columnCount()
 *       - row_id               : sizeof(pos_t)
 *       - table_name           : table_name.size()
 *       - table_name.size()    : sizeof(char)
 *       - transaction_id       : sizeof(transaction_id_t)
 *       - type ("V")           : sizeof(char)
 *
 *     Invalidation Entries:
 *       - invalidated_row_id   : sizeof(pos_t)
 *       - table_name           : table_name.size()
 *       - table_name.size()    : sizeof(char)
 *       - transaction_id       : sizeof(transaction_id_t)
 *       - type ("I")           : sizeof(char)
 *
 *     Commit Entries:
 *       - transaction_id       : sizeof(transaction_id_t)
 *       - type ("C")           : sizeof(char)
 *
 *     Rollback Entries:
 *       - transaction_id       : sizeof(transaction_id_t)
 *       - type ("R")           : sizeof(char)
 *
 *     Skip Entries:
 *       - padding              : bytes filled with 255. Used to align log to BLOCKSIZE (alignment from beginning of
 *file)
 *       - type (255)           : sizeof(char)
 *
 *     Checkpoint Start
 *       - checkpoint id        : sizeof(int)
 *       - type ("X")           : sizeof(char)
 *
 *     Checkpoint End
 *       - checkpoint id        : sizeof(int)
 *       - type ("Y")           : sizeof(char)
 */


#include "io/BufferedLogger.h"
#include "io/StorageManager.h"
#include "io/TransactionManager.h"
#include "storage/Store.h"
#include "storage/ConcurrentUnorderedDictionary.h"

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <helper/Settings.h>
#include <helper/types.h>
#include <helper/dir.h>

#include <iostream>
#include <thread>
#include <boost/filesystem.hpp>

namespace hyrise {
namespace io {

constexpr size_t LOG_BUFFER_CAPACITY = 16384;
constexpr size_t LOG_BLOCKSIZE = 4096;  // Align log to 4KB blocks

BufferedLogger& BufferedLogger::getInstance() {
  static BufferedLogger instance;
  return instance;
}

template <>
void BufferedLogger::logDictionaryValue(char*& cursor, const storage::hyrise_string_t& value) {
  write_string<int>(cursor, value);
  _changes_since_last_checkpoint = true;
}

void BufferedLogger::logInvalidation(const tx::transaction_id_t transaction_id,
                                     const std::string& table_name,
                                     const storage::pos_t invalidated_row) {
  char entry[200];
  char* cursor = entry;
  write_value<storage::pos_t>(cursor, invalidated_row);
  write_string<char>(cursor, table_name);
  write_value<tx::transaction_id_t>(cursor, transaction_id);
  write_value<char>(cursor, 'I');
  size_t len = cursor - entry;
  assert(len <= 200);
  _append(entry, len);
  _changes_since_last_checkpoint = true;
}

void BufferedLogger::logValue(const tx::transaction_id_t transaction_id,
                              const std::string& table_name,
                              const storage::pos_t row,
                              const ValueIdList* value_ids) {
  char entry[200];
  char* cursor = entry;

  if (value_ids != nullptr)
    for (auto it = value_ids->cbegin(); it != value_ids->cend(); ++it)
      write_value<storage::value_id_t>(cursor, it->valueId);
  write_value<storage::pos_t>(cursor, row);
  write_string<char>(cursor, table_name);
  write_value<tx::transaction_id_t>(cursor, transaction_id);
  write_value<char>(cursor, 'V');

  size_t len = cursor - entry;
  assert(len <= 200);
  _append(entry, len);
  _changes_since_last_checkpoint = true;
}

void BufferedLogger::logCommit(const tx::transaction_id_t transaction_id) {
  char entry[9];
  char* cursor = entry;
  write_value<tx::transaction_id_t>(cursor, transaction_id);
  write_value<char>(cursor, 'C');
  _append(entry, 9);
  _changes_since_last_checkpoint = true;
}

void BufferedLogger::logRollback(const tx::transaction_id_t transaction_id) {
  char entry[9];
  char* cursor = entry;
  write_value<tx::transaction_id_t>(cursor, transaction_id);
  write_value<char>(cursor, 'R');
  _append(entry, 9);
  _changes_since_last_checkpoint = true;
}

void BufferedLogger::logStartCheckpoint(const size_t checkpoint_id) {
  char entry[9];
  char* cursor = entry;
  write_value<size_t>(cursor, checkpoint_id);
  write_value<char>(cursor, 'X');
  _append(entry, 9);
}

void BufferedLogger::logEndCheckpoint(const size_t checkpoint_id) {
  char entry[9];
  char* cursor = entry;
  write_value<size_t>(cursor, checkpoint_id);
  write_value<char>(cursor, 'Y');
  _append(entry, 9);
}

size_t BufferedLogger::startCheckpoint() {
  if (_changes_since_last_checkpoint) {
    _changes_since_last_checkpoint = false;

    // wait for running transaction
    tx::TransactionManager::getInstance().waitForAllCurrentlyRunningTransactions();

    _checkpointMutex.lock();
    openNextLogfile();
    logStartCheckpoint(_checkpoint_id);
    return _checkpoint_id;
  } else {
    return 0;
  }
}

size_t BufferedLogger::endCheckpoint() {
  logEndCheckpoint(_checkpoint_id);
  _checkpointMutex.unlock();
  flush();
  writeLastCheckpointID(_checkpoint_id);
  return _checkpoint_id;
}

// write padding entry into reserved write area
void BufferedLogger::writePaddingEntry(size_t absolute_write_pos, size_t padding) {

  auto head = _buffer + (absolute_write_pos % _buffer_capacity);

  if (padding > 0) {
    // clear the padding bytes
    if (head + padding < _tail) {
      memset(head, 255, padding);
    } else {
      size_t part1 = _tail - head;
      size_t part2 = padding - part1;
      memset(head, 255, part1);
      memset(_buffer, 255, part2);
    }
  }
}

char* BufferedLogger::getBufferWriteArea(const size_t size) {

  // get exclusive write are in buffer of size bytes
  auto absolute_write_pos = _total_logsize.fetch_add(size);
  size_t free_space_in_block = LOG_BLOCKSIZE - (absolute_write_pos % LOG_BLOCKSIZE);

  // test if we crash block boundary
  if (free_space_in_block < size) {
    // we do, so we write padding entries in our reserved write area and try again
    // write padding for end of first block
    writePaddingEntry(absolute_write_pos, free_space_in_block);

    // write padding for start of second block
    writePaddingEntry(absolute_write_pos + free_space_in_block, size - free_space_in_block);

    // add padding to buffer size
    _buffer_size.fetch_add(size);

    // request new write area
    return getBufferWriteArea(size);
  }

  _buffer_size.fetch_add(size);

  return _buffer + (absolute_write_pos % _buffer_capacity);
}


void BufferedLogger::_append(const char* str, const unsigned char len) {

  // we need one char extra at the beginning to indicate if writing the block is finished
  auto write_size = len + 1;

  // request write area
  char* head = getBufferWriteArea(write_size);

  // do not write the first byte
  ++head;

  // write the actual entry
  if (head + len < _tail) {
    memcpy(head, str, len);
  } else {
    size_t part1 = _tail - head;
    size_t part2 = len - part1;
    memcpy(head, str, part1);
    memcpy(_buffer, str + part1, part2);
  }

  // set first byte to size of entry
  // as this is not zero, it indicates that writing the block is finished
  --head;
  *head = len;

  // initiate a flush of the buffer if it is filled to more than 50%
  // but do not block if flush is aready in progress
  if (_buffer_size.load() > _buffer_capacity / 2) {
    flush(false);
  }
}

void BufferedLogger::flush(bool blocking) {

  // only one flush at a time. aqcuire the lock.
  // if blocking is false we give up if the lock could not be aquired
  if (blocking == false) {
    if (_flushMutex.try_lock() == false) {
      return;
    }
  } else {
    _flushMutex.lock();
  }

  // get area that is save for flushing, meaning all entries are finished writing
  auto flush_barrier_pos = _last_flush_pos;

  while (true) {

    // read the first byte
    unsigned char value = getBufferValueAtPos(flush_barrier_pos);

    if (value == 0) {
      // entry is not finished, can't flush it yet. so we stop here
      break;
    } else if (value == 255) {
      // entry is skip entry, we move forward by one
      ++flush_barrier_pos;
    } else {
      // value is between 0 and 255
      // this means it indicates the size of a written entry
      // we move forward by those byte
      flush_barrier_pos += value + 1;
    }
  }

  // get the file mutex so we can't move to the next logfile in parallel
  _fileMutex.lock();

  // write buffer to file and clear bytes in buffer
  size_t written = 0;
  auto flush_barrier = getBufferPointerAtPos(flush_barrier_pos);
  auto last_flush = getBufferPointerAtPos(_last_flush_pos);
  if (flush_barrier > last_flush) {
    written = flush_barrier - last_flush;
    fwrite(last_flush, sizeof(char), written, _logfile);
    memset(last_flush, 0, written);
  } else if (flush_barrier < last_flush) {
    size_t part1 = _tail - last_flush;
    size_t part2 = flush_barrier - _buffer;
    written = part1 + part2;
    fwrite(last_flush, sizeof(char), part1, _logfile);
    fwrite(_buffer, sizeof(char), part2, _logfile);
    memset(last_flush, 0, part1);
    memset(_buffer, 0, part2);
  }

  _buffer_size -= written;
  _last_flush_pos = flush_barrier_pos;

  // #ifndef COMMIT_WITHOUT_FLUSH
  if (fflush(_logfile) != 0) {
    printf("Something went wrong while flushing the logfile: %s\n", strerror(errno));
  }
  if (fsync(fileno(_logfile)) != 0) {
    printf("Something went wrong while fsyncing the logfile: %s\n", strerror(errno));
  }
  // #endif

  _fileMutex.unlock();
  _flushMutex.unlock();
}

std::string BufferedLogger::getLogfilenameForCheckpoint(size_t checkpoint_id) {
  std::stringstream ss;
  ss << _logdir << std::setw(5) << std::setfill('0') << _checkpoint_id << ".bin";
  return ss.str();
}

void BufferedLogger::openNextLogfile() {
  _fileMutex.lock();
  ++_checkpoint_id;

  auto logfilename = getLogfilenameForCheckpoint(_checkpoint_id);
  if (_logfile)
    fclose(_logfile);
  _logfile = fopen(logfilename.c_str(), "a");
  if (_logfile == NULL)
    throw std::runtime_error("Could not open logfile: " + logfilename);

  // fsync the file
  auto logfile_file_int_descriptor = fileno(_logfile);
  if (logfile_file_int_descriptor == -1) {
    printf("Something went wrong while getting the file descriptor for the _logfile for fsyncing: %s\n",
           strerror(errno));
  }

  auto fsync_file_result = fsync(logfile_file_int_descriptor);
  if (fsync_file_result == -1) {
    printf("Something went wrong while fsyncing the log file: %s\n", strerror(errno));
  }

  // fsync the directory
  auto log_dir = opendir(_logdir.c_str());
  if (log_dir == NULL) {
    printf("Something went wrong while opening the log directory for fsyncing: %s\n", strerror(errno));
  }

  auto log_dir_fd = dirfd(log_dir);
  if (log_dir_fd == -1) {
    printf(
        "Something went wrong while getting the  directory stream file descriptor for the log directory for fsyncing: "
        "%s\n",
        strerror(errno));
  }

  auto fsync_dir_result = fsync(log_dir_fd);
  if (fsync_dir_result == -1) {
    printf("Something went wrong while fsyncing the log directory: %s\n", strerror(errno));
  }

  // close the directory
  if (log_dir)
    closedir(log_dir);

  struct stat s;
  stat(logfilename.c_str(), &s);

  _total_logsize += s.st_size;
  _fileMutex.unlock();
}

size_t BufferedLogger::readLastCheckpointID() {
  // read all empty checkpoint files and return highest id, which is the last successfully finished checkpoint
  auto files = _listdir(Settings::getInstance()->getCheckpointDir());
  size_t lastid = 0;
  for (auto filename : files) {
    if (filename.size() > 4 && filename.compare(0, 2, "__") == 0 &&
        filename.compare(filename.size() - 2, 2, "__") == 0) {
      auto str_id = filename.substr(2, filename.size() - 4);
      size_t id = std::stoi(str_id);

      if (id > lastid) {
        lastid = id;
      }
    }
  }
  return lastid;
}

void BufferedLogger::writeLastCheckpointID(size_t checkpoint_id) {
  // every finished checkpoint is marked by writing an empty file called __id__
  std::string checkpoint_filename =
      Settings::getInstance()->getCheckpointDir() + "__" + std::to_string(checkpoint_id) + "__";
  auto checkpoint_file = fopen(checkpoint_filename.c_str(), "a");

  // fsync the file
  auto checkpoint_file_int_descriptor = fileno(checkpoint_file);
  if (checkpoint_file_int_descriptor == -1) {
    printf("Something went wrong while getting the file descriptor for the checkpoint_file for fsyncing: %s\n",
           strerror(errno));
  }

  auto fsync_file_result = fsync(checkpoint_file_int_descriptor);
  if (fsync_file_result == -1) {
    printf("Something went wrong while fsyncing the checkpointing file: %s\n", strerror(errno));
  }

  // fsync the directory
  auto checkpoint_dir = opendir(Settings::getInstance()->getCheckpointDir().c_str());
  if (checkpoint_dir == NULL) {
    printf("Something went wrong while opening the checkpointing directory for fsyncing: %s\n", strerror(errno));
  }

  auto checkpoint_dir_fd = dirfd(checkpoint_dir);
  if (checkpoint_dir_fd == -1) {
    printf(
        "Something went wrong while getting the  directory stream file descriptor for the checkpointing directory for "
        "fsyncing: %s\n",
        strerror(errno));
  }

  auto fsync_dir_result = fsync(checkpoint_dir_fd);
  if (fsync_dir_result == -1) {
    printf("Something went wrong while fsyncing the checkpointing directory: %s\n", strerror(errno));
  }

  // close the directory
  if (checkpoint_dir)
    closedir(checkpoint_dir);

  // close the file
  if (checkpoint_file)
    fclose(checkpoint_file);
}

BufferedLogger::BufferedLogger() {
  _logdir = Settings::getInstance()->getLogDir();
  _checkpoint_id = readLastCheckpointID();
  openNextLogfile();

  _total_logsize = 0;
  _buffer_capacity = LOG_BUFFER_CAPACITY;
  _buffer = (char*)malloc(_buffer_capacity);
  _head = _buffer;
  _last_flush_pos = 0;
  _tail = _buffer + _buffer_capacity;
  _buffer_size = 0;
  _changes_since_last_checkpoint = true;

  // clear the complete buffer
  memset(_buffer, 0, _buffer_capacity);
}

void BufferedLogger::truncate() {
  // clears all logs and checkpoints
  _checkpoint_id = 0;
  boost::filesystem::remove_all(Settings::getInstance()->getLogDir());
  boost::filesystem::remove_all(Settings::getInstance()->getCheckpointDir());
  boost::filesystem::remove_all(Settings::getInstance()->getTableDumpDir());
  _mkdir(Settings::getInstance()->getLogDir());
  _mkdir(Settings::getInstance()->getCheckpointDir());
  _mkdir(Settings::getInstance()->getTableDumpDir());

  openNextLogfile();
  _head = _buffer;
  _last_flush_pos = 0;
  _buffer_size = 0;
  _total_logsize = 0;

  // clear the complete buffer
  memset(_buffer, 0, _buffer_capacity);
}

void BufferedLogger::restore(const size_t thread_count) {

  auto logfilename = getLogfilenameForCheckpoint(_checkpoint_id);

  int fd = open(logfilename.c_str(), O_RDWR);
  assert(fd);

  struct stat s;
  fstat(fd, &s);

  auto logfile = (char*)mmap(0, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

  std::vector<bool> committed_tid_bitvector;
  std::vector<bool> rolledback_tid_bitvector;
  committed_tid_bitvector.resize(1000000, false);  // FIXME: somehow identify largest TID upfront
  rolledback_tid_bitvector.resize(1000000, false);  // FIXME: somehow identify largest TID upfront

  std::vector<std::thread> threads;
  size_t number_of_blocks_in_log = s.st_size / LOG_BLOCKSIZE;
  size_t leftover_bytes = s.st_size % LOG_BLOCKSIZE;
  size_t blocks_per_thread = number_of_blocks_in_log / thread_count;
  size_t leftover_blocks = number_of_blocks_in_log % thread_count;

  thread_barrier barrier(thread_count);

  // size_t *thread_results = (size_t*)malloc(sizeof(size_t) * thread_count);

  for (size_t thread_id = 0; thread_id < thread_count; ++thread_id) {
    size_t start_block = thread_id * blocks_per_thread;
    size_t end_block = (thread_id + 1) * blocks_per_thread;
    size_t leftovers = 0;

    if (thread_id == thread_count - 1) {
      end_block += leftover_blocks;
      leftovers = leftover_bytes;
    }

    threads.push_back(std::thread(&BufferedLogger::restore_thread,
                                  this,
                                  logfile,
                                  start_block,
                                  end_block,
                                  leftovers,
                                  thread_id,
                                  std::ref(committed_tid_bitvector),
                                  std::ref(rolledback_tid_bitvector),
                                  std::ref(barrier)));
  }

  for (auto& thread : threads) {
    thread.join();
  }

  // close the file
  if (fd)
    close(fd);
}

void BufferedLogger::restore_thread(char* logfile,
                                    size_t start_block,
                                    size_t end_block,
                                    size_t leftovers,
                                    size_t thread_id,
                                    std::vector<bool>& committed_tid_bitvector,
                                    std::vector<bool>& rolledback_tid_bitvector,
                                    thread_barrier& barrier) {

  std::vector<std::tuple<storage::store_ptr_t, tx::transaction_id_t, storage::pos_t>> pending_inserts;
  std::vector<std::tuple<storage::store_ptr_t, tx::transaction_id_t, storage::pos_t>> pending_deletes;

  // start reading the logfile from the back
  char* cursor = logfile + (end_block * LOG_BLOCKSIZE) - 1 + leftovers;

  // while not reaching start of file, continue reading
  while (cursor >= logfile + (start_block * LOG_BLOCKSIZE)) {

    auto entry_type = read_value<char>(cursor);
    switch (entry_type) {
      case 'D': {
        auto table_name = read_string<char>(cursor);
        auto column = read_value<storage::field_t>(cursor);
        auto value_id = read_value<storage::value_id_t>(cursor);
        auto store = std::dynamic_pointer_cast<storage::Store>(StorageManager::getInstance()->getTable(table_name));

        switch (store->typeOfColumn(column)) {
          // kill me...
          case IntegerType:
          case IntegerTypeDelta:
          case IntegerTypeDeltaConcurrent: {
            read_value<int>(cursor);  // skip
            auto dict = std::dynamic_pointer_cast<storage::ConcurrentUnorderedDictionary<storage::hyrise_int_t>>(
                store->getDeltaTable()->dictionaryAt(column));
            auto value = read_value<storage::hyrise_int_t>(cursor);
            assert(dict);
            dict->setValueId(value, value_id);
            break;
          }
          case FloatType:
          case FloatTypeDelta:
          case FloatTypeDeltaConcurrent: {
            read_value<int>(cursor);  // skip
            auto dict = std::dynamic_pointer_cast<storage::ConcurrentUnorderedDictionary<storage::hyrise_float_t>>(
                store->getDeltaTable()->dictionaryAt(column));
            auto value = read_value<storage::hyrise_float_t>(cursor);
            assert(dict);
            dict->setValueId(value, value_id);
            break;
          }
          case StringType:
          case StringTypeDelta:
          case StringTypeDeltaConcurrent: {
            auto dict = std::dynamic_pointer_cast<storage::ConcurrentUnorderedDictionary<storage::hyrise_string_t>>(
                store->getDeltaTable()->dictionaryAt(column));
            auto value = read_string<int>(cursor);
            assert(dict);
            dict->setValueId(value, value_id);
            break;
          }
          default:
            throw std::runtime_error("Unsupported column type for recovery.");
        }

        read_value<char>(cursor);  // finished flag only used for flushing
        break;
      }

      // insert of a new row with values
      case 'V': {
        auto transaction_id = read_value<tx::transaction_id_t>(cursor);
        auto table_name = read_string<char>(cursor);
        auto store = std::dynamic_pointer_cast<storage::Store>(StorageManager::getInstance()->getTable(table_name));
        auto row = read_value<storage::pos_t>(cursor);
        auto pos_in_delta = row - store->deltaOffset();
        auto value_id = 0;

        if (row >= store->size())
          store->appendToDelta(row - store->size() + 1);

        for (size_t i = 1; i <= store->columnCount(); i++) {
          value_id = read_value<storage::value_id_t>(cursor);
          // columns were logged in ascending order - we get the last column first
          store->getDeltaTable()->setValueId(store->columnCount() - i, pos_in_delta, ValueId(value_id, 1));
        }

        // add row to indices
        store->addRowToDeltaIndices(row);

        // handle invalidated row
        if (committed_tid_bitvector[transaction_id] == true) {
          store->setBeginCid(row, tx::START_TID);
        }

        if (committed_tid_bitvector[transaction_id] == true) {
          // Transaction was committed so we set the row to valid
          store->setBeginCid(row, tx::START_TID);
        } else if (rolledback_tid_bitvector[transaction_id] == false) {
          // We have not found a commit or rollback entry for the TX,
          // so we save it for separate reprocessing
          pending_inserts.push_back(std::make_tuple(store, transaction_id, row));
        }  // else: Insert was rolled back, so nothing to do here

        read_value<char>(cursor);  // finished flag only used for flushing
        break;
      }

      // invalidated row
      case 'I': {
        auto transaction_id = read_value<tx::transaction_id_t>(cursor);
        auto table_name = read_string<char>(cursor);
        auto store = std::dynamic_pointer_cast<storage::Store>(StorageManager::getInstance()->getTable(table_name));
        auto invalidated_row = read_value<storage::pos_t>(cursor);

        if (committed_tid_bitvector[transaction_id] == true) {
          // Transaction was committed so we invalidate the row
          if (invalidated_row >= store->size())
            store->appendToDelta(invalidated_row - store->size() + 1);
          store->setEndCid(invalidated_row, tx::START_TID);
        } else if (rolledback_tid_bitvector[transaction_id] == false) {
          // We have not found a commit or rollback entry for the TX,
          // so we save it for separate reprocessing
          pending_deletes.push_back(std::make_tuple(store, transaction_id, invalidated_row));
        }  // else: Delete was rolled back, so nothing to do here

        read_value<char>(cursor);  // finished flag only used for flushing
        break;
      }

      // commited transaction
      case 'C': {
        auto transaction_id = read_value<tx::transaction_id_t>(cursor);
        committed_tid_bitvector[transaction_id] = true;
        read_value<char>(cursor);  // finished flag only used for flushing
        break;
      }

      // rolled back transaction
      case 'R': {
        auto transaction_id = read_value<tx::transaction_id_t>(cursor);
        rolledback_tid_bitvector[transaction_id] = true;
        read_value<char>(cursor);  // finished flag only used for flushing
        break;
      }

      // Checkpoint start
      case 'X': {
        read_value<size_t>(cursor);
        read_value<char>(cursor);  // finished flag only used for flushing
        break;
      }

      // Checkpoint end
      case 'Y': {
        read_value<size_t>(cursor);
        read_value<char>(cursor);  // finished flag only used for flushing
        break;
      }

      default: {
        // is it a skip entry filled with padding?
        if ((unsigned char)entry_type == 255) {
          // skip padding by skipping all bytes until next byte is non-zero
          // padding entry has no finished flag
          while (*(unsigned char*)cursor == 255)
            cursor -= sizeof(entry_type);
          break;
        } else {
          // no idea what this should be :(
          throw std::runtime_error("Unsupported log entry type for recovery.");
        }
        break;
      }
    }
  }

  // synchronize all threads
  barrier.wait();

  for (auto pending : pending_inserts) {
    auto store = std::get<0>(pending);
    auto transaction_id = std::get<1>(pending);
    auto row = std::get<2>(pending);

    if (committed_tid_bitvector[transaction_id] == true) {
      // Transaction was committed so we set the row to valid
      store->setBeginCid(row, tx::START_TID);
    } else {
      // We have not found a commit entry for the TX,
      // so the row gets invalidated
      store->setBeginCid(row, tx::INF_CID);
    }
  }

  for (auto pending : pending_deletes) {
    auto store = std::get<0>(pending);
    auto transaction_id = std::get<1>(pending);
    auto row = std::get<2>(pending);

    if (committed_tid_bitvector[transaction_id] == true) {
      // The delete was committed so we invalidate the row
      store->setEndCid(row, tx::START_TID);
    }
  }
}
}
}
