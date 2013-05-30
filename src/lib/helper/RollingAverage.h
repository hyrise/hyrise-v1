/*
 * RollingAverage.h
 *
 *  Created on: Jul 25, 2012
 *      Author: jwust
 */
#include <vector>

#ifndef ROLLINGAVERAGE_H_
#define ROLLINGAVERAGE_H_

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

#endif /* ROLLINGAVERAGE_H_ */
