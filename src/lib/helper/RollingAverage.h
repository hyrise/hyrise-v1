// Copyright (c) 2013 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <vector>

template <class T>
class RollingAverage {
	size_t _sample_size;
	size_t _oldest;
	T _total;
	std::vector<T> _sample;
public:
	RollingAverage(int sample): _sample_size(sample), _oldest(0), _total(0){
	};
	T add(T value){
		if (_sample_size == _sample.size())
		{
			_total -= _sample[_oldest];
			_sample[_oldest] = value;
		} else {
			_sample.push_back(value);
		}
		_total += value;
		_oldest = (_oldest + 1) % _sample_size;
		return _total/_sample.size();
	}
	void set(T value){
	  for(size_t i = 0; i < _sample_size; i++)
	    add(value);
	}

	void reset(){
	  _sample.clear();
	  _oldest = 0;
	  _total = 0;
	}

	void clear(){
		_total = 0;
		_sample.clear();
		_oldest = 0;
	}
	T getAverage() const{
		T average = 0;
		if(_sample.size() > 0)
			average = _total/_sample.size();
		return average;
	}
};

