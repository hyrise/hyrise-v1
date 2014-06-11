// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "Settings.h"
#include "Environment.h"
#include "dir.h"

#include <stdexcept>
#include <iostream>

Settings::Settings() : threadpoolSize(1) {

  // Initiate the class based on Enviroment Variables
  setDBPath(getEnv("HYRISE_DB_PATH", ""));
  setScriptPath(getEnv("HYRISE_SCRIPT_PATH", "test/script"));
  setProfilePath(getEnv("HYRISE_PROFILE_PATH", "."));

  if (getDBPath().empty()) {
    std::cout << "No DB-Path specified, using current working directory..." << std::endl;
    setDBPath(_current_working_dir());
  }

  _mkdir(getPersistencyDir());
  _mkdir(getLogDir());
  _mkdir(getTableDumpDir());
  _mkdir(getCheckpointDir());
}

size_t Settings::getThreadpoolSize() const { return this->threadpoolSize; }

void Settings::setThreadpoolSize(const size_t newSize) { this->threadpoolSize = newSize; }

void Settings::printInfo() {
  std::string del = "  ";
  std::cout << std::endl << "Hyrise server running with the following settings:" << std::endl;
  std::cout << del << "DB Path: " << getDBPath() << std::endl;
  std::cout << del << "Scheduler: " << scheduler_name << std::endl;
  std::cout << del << "Worker Threads: " << worker_threads << std::endl;
  std::cout << del << "Port:" << port << std::endl;

#ifdef PERSISTENCY_NONE
  std::cout << del << "Persistency: None" << std::endl;
#endif
#ifdef PERSISTENCY_BUFFEREDLOGGER
  std::cout << del << "Persistency: Logger" << std::endl;
  std::cout << del << "Commit-Window [ms]: " << commit_window_ms << std::endl;
  std::cout << del << "Checkpoint Interval [ms]: " << checkpoint_interval << std::endl;
  std::cout << del << "Persistency Directory: " << getDBPath() << std::endl;
#endif

  std::cout << std::endl;
}
