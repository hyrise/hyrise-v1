// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <iostream>

class Progress {

 private:
  size_t _steps;
  size_t _current_step;
  size_t _current_tick;
  size_t _ticks;

 public:

  explicit Progress(size_t ticks) : _steps(50), _current_step(0), _current_tick(0),  _ticks(ticks) {
    if (ticks < _steps) {
      _steps = ticks;
    }

    if (ticks == 0) {
      _current_step = 100;
    }

    tick();
  }

  void tick() {
    _current_tick++;

    if (_current_tick >= _current_step * _ticks / 100) {
      std::cout << _current_step << "." << std::flush;

      if (_current_step == 100) {
        std::cout << std::endl;
        return;
      }

      _current_step += 100 / _steps;
    }
  }

};

