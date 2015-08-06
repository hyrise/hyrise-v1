// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once
#include <mutex>
#include <condition_variable>

class thread_barrier {
 private:
  std::mutex m_mutex;
  std::condition_variable m_cond;
  unsigned int m_threshold;
  unsigned int m_count;
  unsigned int m_generation;

 public:
  thread_barrier(unsigned int count) : m_threshold(count), m_count(count), m_generation(0) {}

  bool wait() {
    std::unique_lock<std::mutex> lock(m_mutex);
    unsigned int gen = m_generation;

    if (--m_count == 0) {
      m_generation++;
      m_count = m_threshold;
      m_cond.notify_all();
      return true;
    }

    while (gen == m_generation)
      m_cond.wait(lock);
    return false;
  }
};
