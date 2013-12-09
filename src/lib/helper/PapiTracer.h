// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <stdio.h>
#include <sys/time.h>

#include <algorithm>
#include <vector>
#include <string>
#include <stdexcept>

#include "helper/stringhelpers.h"

/// For errors generally related to tracing
class TracingError : public std::runtime_error {
 public:
  TracingError(std::string what) : std::runtime_error("TracingError: " + what) {}
};

#ifdef USE_PAPI_TRACE
#include <mutex>

#include "helper/locking.h"

#include "papi.h"
/// Tracing wrapper for PAPI
///
/// Usage:
///
///     PapiTracer pt;
///     pt.addEvent("PAPI_TOT_INS"); // add counter for total instructions
///     pt.start();
///     /* do some work */
///     pt.stop();
///     std::cout << pt.value("PAPI_TOT_INS") << std::endl;
///
/// Multiple events can be added but may or may not work due to restrictions
/// of the underlying hardware.
class PapiTracer {
 public:
  typedef long long result_t;
 private:
  //! the initialized eventset from PAPI
  int _eventSet;
  bool _disabled;
  bool _running;
  //! keeps track of registered counters
  std::vector<std::string> _counters;
  //! performance counter results
  std::vector<result_t> _results;

  /// Shorthand for error testing with PAPI functions
  ///
  /// @param[in] function PAPI function to call that returns PAPI error codes
  /// @param[in] activity Information about the activity currently conducted, for error reporting
  /// @param[in] args parameters of the function to call
  template<typename Func, typename ActivityT, typename... Args>
  static void handle(Func function, ActivityT activity, Args&&... args) {
    int retval = function(std::forward<Args>(args)...);
    if (retval != PAPI_OK)
      throw TracingError(std::string(activity) + " failed: " + PAPI_strerror(retval));
  }

  static void initialize() {
    static bool initialized = false;
    static hyrise::locking::Spinlock init_mtx;

    std::lock_guard<decltype(init_mtx)> guard(init_mtx);
    if (!initialized) {
      if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT)
        throw TracingError("PAPI could not be initialized");
      initialized = true;
    }
  }

 public:
  inline PapiTracer() : _eventSet(PAPI_NULL), _disabled(false), _running(false) {
    PapiTracer::initialize();
  }

  inline ~PapiTracer() {
    stop();
  }

  /// Add a new event counter
  ///
  /// @param[in] eventName valid PAPI event name (see `$ papi_avail`)
  inline void addEvent(std::string eventName) {
    if (eventName == "NO_PAPI" || _disabled) {
      _disabled = true;
      return;
    }
    _counters.push_back(eventName);
  }

  /// Start performance counter
  inline void start() {
    if (_disabled)
      return;

    handle(PAPI_thread_init, "Initialize thread", pthread_self);
    handle(PAPI_create_eventset, "Eventset creation", &_eventSet);

    if (_counters.empty())
      throw std::runtime_error("No events set");

    for(const auto& eventName: _counters) {
      handle(PAPI_add_named_event,
             "Adding event " + eventName + " to event set",
             _eventSet, (char*) eventName.c_str());
    }

    _results.clear();
    _results.resize(_counters.size());

    _running = true;

    //handle(PAPI_reset, "Reset counter", _eventSet);
    handle(PAPI_start, "Starting counter", _eventSet);
  }

  /// Stop performance counter
  inline void stop() {
    if (_disabled || !_running) return;

    handle(PAPI_stop, "Stopping Counter", _eventSet, _results.data());

    handle(PAPI_cleanup_eventset, "cleaning eventset", _eventSet);
    handle(PAPI_destroy_eventset, "destroying eventset", &_eventSet);

    _running = false;
  }

  /// Retrieve performance counter values
  inline long long value(const std::string& eventName) const {
    if (_disabled) return 0ull;

    auto item = std::find(_counters.begin(), _counters.end(), eventName);
    if (item == _counters.end())
      throw TracingError("Trying to access unregistered event '" + eventName +"' "
                         "Available: " + joinString(_counters, " "));
    auto index = std::distance(_counters.begin(), item);
    return _results.at(index);
  }

  static bool isPapi() { return true; }
};

#else

/// Fallback tracing mechanism that behaves like PapiTracer
/// concerning adding of new events to measure but will
/// only return the time elapsed time in microseconds. This is useful
/// for systems without PAPI support such as virtual machines
///
/// @note This tracer does not check for PAPI event name validity!
class FallbackTracer {
 public:
  typedef long long result_t;
 private:
  std::vector<std::string> _counters;
  result_t _result;
  struct timeval _start;
 public:
  inline void addEvent(std::string eventName) {
    _counters.push_back(eventName);
  }

  inline void start() {
    if (_counters.empty())
      throw TracingError("No events set");
    _result = 0;
    gettimeofday(&_start, nullptr);
  }

  inline void stop() {
    struct timeval end = {0, 0};
    gettimeofday(&end, nullptr);
    _result = (end.tv_sec - _start.tv_sec) * 1000000 + (end.tv_usec - _start.tv_usec);
  }

  inline long long value(const std::string& eventName) const {
    auto item = find(_counters.begin(), _counters.end(), eventName);
    if (item == _counters.end())
      throw TracingError("Trying to access unregistered event '" + eventName +"' "
                         "Available: " + joinString(_counters, " "));
    return _result;
  }
  static bool isPapi() { return false; }
};

typedef FallbackTracer PapiTracer;

#endif

