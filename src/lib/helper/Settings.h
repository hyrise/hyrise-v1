// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#define ADD_MEMBER(type, member)                 \
 private:                                        \
  type _##member;                                \
                                                 \
 public:                                         \
  type get##member() const { return _##member; } \
  void set##member(const type nm) { _##member = nm; }



/*  Singleton data container class for global settings.
    Use SettingsOperation to manipulate global settings as long as units
    implementing certain decisions are missing. */

class Settings {


  size_t threadpoolSize;

  ADD_MEMBER(std::string, ScriptPath);
  ADD_MEMBER(std::string, ProfilePath);
  ADD_MEMBER(std::string, DBPath);

  Settings();

 public:
  ~Settings() {}
  static Settings* getInstance() {
    static Settings* instance = nullptr;
    if (instance == nullptr)
      instance = new Settings();
    return instance;
  }

  //  Control the maximum number of parallel executable operation tasks.
  size_t getThreadpoolSize() const;
  void setThreadpoolSize(const size_t newSize);
  void printInfo();

  size_t worker_threads;
  size_t port;
  size_t checkpoint_interval;
  std::vector<size_t> numa_nodes, numa_cores;
  std::string scheduler_name;
  size_t commit_window_ms;

  std::string getPersistencyDir() {
    return getDBPath() + "/persistency/";
  };
  std::string getLogDir() {
    return getPersistencyDir() + "/logs/";
  };
  std::string getTableDumpDir() {
    return getPersistencyDir() + "/tables/";
  };
  std::string getCheckpointDir() {
    return getPersistencyDir() + "/checkpoints/";
  };
};
