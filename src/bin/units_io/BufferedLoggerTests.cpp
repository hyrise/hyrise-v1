#include "testing/test.h"
#include "io/BufferedLogger.h"
#include "io/shortcuts.h"
#include "io/StorageManager.h"
#include "storage/Store.h"
#include "io/TransactionManager.h"
#include "access/InsertScan.h"
#include "access/PosUpdateScan.h"
#include "access/tx/Commit.h"
#include "access/tx/Rollback.h"
#include "access/Checkpoint.h"
#include "storage/PointerCalculator.h"


#include <fstream>
#include <thread>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <helper/Settings.h>

namespace hyrise {
namespace io {

class BufferedLoggerTests : public ::hyrise::Test {};

TEST_F(BufferedLoggerTests, log_test) {
  BufferedLogger::getInstance().truncate();
  uint64_t tx = 17;
  std::string table_name("BLABLA");

  BufferedLogger::getInstance().logDictionary<int64_t>(table_name, 1, 2, 31);
  BufferedLogger::getInstance().logDictionary<float>(table_name, 2, 2.0f, 42);
  BufferedLogger::getInstance().logDictionary<std::string>(table_name, 3, "zwei", 53);
  std::vector<ValueId> vids;
  vids.push_back(ValueId(31, 1));
  vids.push_back(ValueId(42, 2));
  vids.push_back(ValueId(53, 3));
  BufferedLogger::getInstance().logValue(tx, table_name, 3, &vids);
  BufferedLogger::getInstance().logInvalidation(tx, table_name, 2);
  BufferedLogger::getInstance().logCommit(tx);
  BufferedLogger::getInstance().flush();

  int log_fd = open(BufferedLogger::getInstance().getLogfilenameForCheckpoint(1).c_str(), O_RDONLY);
  int reference_fd = open((Settings::getInstance()->getDBPath() + "/buffered_logger_logfile.bin").c_str(), O_RDONLY);
  ASSERT_TRUE(log_fd);
  ASSERT_TRUE(reference_fd);

  // struct stat log_s, reference_s;
  // fstat(log_fd, &log_s);
  // fstat(reference_fd, &reference_s);
  // ASSERT_EQ(log_s.st_size, reference_s.st_size);

  // auto log = (char*)mmap(NULL, log_s.st_size, PROT_READ, MAP_PRIVATE, log_fd, 0);
  // auto reference = (char*)mmap(NULL, reference_s.st_size, PROT_READ, MAP_PRIVATE, reference_fd, 0);

  // ASSERT_FALSE(memcmp(log, reference, log_s.st_size));
}

#ifdef PERSISTENCY_BUFFEREDLOGGER
TEST_F(BufferedLoggerTests, insert_and_restore_test) {
  BufferedLogger::getInstance().truncate();

  auto rows = Loader::shortcuts::load("test/alltypes.tbl");
  auto orig = Loader::shortcuts::load("test/alltypes_empty.tbl");

  orig->setName("TABELLE");
  StorageManager::getInstance()->add("TABELLE", orig);
  StorageManager::getInstance()->persistTable("TABELLE");

  for (size_t i = 0; i < 1000; ++i) {
    auto ctx = tx::TransactionManager::getInstance().buildContext();
    access::InsertScan is;
    is.setTXContext(ctx);
    is.addInput(orig);
    is.setInputData(rows);
    is.execute();

    auto ctx2 = tx::TransactionManager::getInstance().buildContext();
    access::InsertScan is2;
    is2.setTXContext(ctx2);
    is2.addInput(orig);
    is2.setInputData(rows);
    is2.execute();

    auto ctx3 = tx::TransactionManager::getInstance().buildContext();
    access::InsertScan is3;
    is3.setTXContext(ctx3);
    is3.addInput(orig);
    is3.setInputData(rows);
    is3.execute();

    access::Commit c3;
    c3.addInput(orig);
    c3.setTXContext(ctx3);
    c3.execute();

    access::Rollback r2;
    r2.addInput(orig);
    r2.setTXContext(ctx2);
    r2.execute();

    access::Commit c;
    c.addInput(orig);
    c.setTXContext(ctx);
    c.execute();
  }

  StorageManager::getInstance()->removeTable("TABELLE");

  storage::atable_ptr_t restored(new storage::Store(rows->copy_structure_modifiable()));
  StorageManager::getInstance()->add("TABELLE", restored);

  BufferedLogger::getInstance().restore(1);

  ASSERT_TABLE_EQUAL(orig, restored);

  StorageManager::getInstance()->removeTable("TABELLE");
}

TEST_F(BufferedLoggerTests, pos_update_and_restore_test) {
  BufferedLogger::getInstance().truncate();

  auto rows = Loader::shortcuts::load("test/alltypes.tbl");
  auto orig = Loader::shortcuts::load("test/alltypes_empty.tbl");

  orig->setName("TABELLE");
  StorageManager::getInstance()->add("TABELLE", orig);
  StorageManager::getInstance()->persistTable("TABELLE");

  auto ctx = tx::TransactionManager::getInstance().buildContext();
  access::InsertScan is;
  is.setTXContext(ctx);
  is.addInput(orig);
  is.setInputData(rows);
  is.execute();

  access::PosUpdateScan pos;
  // create PC to simulate position
  auto pc = storage::PointerCalculator::create(orig, new pos_list_t({1}));

  pos.setTXContext(ctx);
  pos.addInput(pc);
  Json::Value v;
  v["col_int"] = 12;
  v["col_string"] = "foobar";
  pos.setRawData(v);
  pos.execute();

  access::Commit c;
  c.addInput(orig);
  c.setTXContext(ctx);
  c.execute();

  StorageManager::getInstance()->removeTable("TABELLE");

  storage::atable_ptr_t restored(new storage::Store(orig->copy_structure_modifiable()));
  StorageManager::getInstance()->add("TABELLE", restored);

  BufferedLogger::getInstance().restore(1);

  ASSERT_TABLE_EQUAL(orig, restored);

  StorageManager::getInstance()->removeTable("TABELLE");
}


TEST_F(BufferedLoggerTests, simple_checkpoint_test) {
  BufferedLogger::getInstance().truncate();

  auto rows = Loader::shortcuts::load("test/alltypes.tbl");
  auto orig = Loader::shortcuts::load("test/alltypes.tbl");

  const std::string tablename = "CHECKPOINT_TEST";
  auto sm = StorageManager::getInstance();

  orig->setName(tablename);
  sm->add(tablename, orig);

  // first checkpoint with main
  access::Checkpoint cp;
  cp.setWithMain(true);
  cp.execute();

  // insert some rows
  auto ctx = tx::TransactionManager::getInstance().buildContext();
  access::InsertScan is;
  is.setTXContext(ctx);
  is.addInput(orig);
  is.setInputData(rows);
  is.execute();

  access::Commit c;
  c.addInput(is.getResultTable());
  c.setTXContext(ctx);
  c.execute();

  // do delta checkpoint
  access::Checkpoint cp2;
  cp2.setWithMain(false);
  cp2.execute();

  // insert some more rows
  auto ctx2 = tx::TransactionManager::getInstance().buildContext();
  access::InsertScan is2;
  is2.setTXContext(ctx2);
  is2.addInput(orig);
  is2.setInputData(rows);
  is2.execute();

  access::Commit c2;
  c2.addInput(is2.getResultTable());
  c2.setTXContext(ctx2);
  c2.execute();

  sm->removeTable(tablename);
  sm->recoverTables();

  ASSERT_TRUE(sm->exists(tablename));
  auto restored = sm->getTable(tablename);
  ASSERT_TABLE_EQUAL(orig, restored);
  StorageManager::getInstance()->removeTable(tablename);
}
#endif
}
}
