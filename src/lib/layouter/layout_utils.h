// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <set>
#include <vector>

namespace hyrise {
namespace layouter {

// Globale subset_t definition
typedef std::vector<unsigned> subset_t;
typedef std::set<unsigned> set_subset_t;


// Helper methods to print all different kind of subsets and sets
// and layouts
void print(std::set<unsigned> x);
void print(std::vector<unsigned> x);
void print(std::vector<std::vector<unsigned> > x);
void print(std::vector<double> x);
void print(int *x, int s);

} } // namespace hyrise::layouter

// Copy Helper
std::vector<hyrise::layouter::subset_t> &operator += (std::vector<hyrise::layouter::subset_t> &a, const std::vector<hyrise::layouter::subset_t> &b);
std::vector<hyrise::layouter::subset_t>   operator + (const std::vector<hyrise::layouter::subset_t> &a, const std::vector<hyrise::layouter::subset_t> &b);

bool operator== (const std::vector<hyrise::layouter::subset_t> &left, const std::vector<hyrise::layouter::subset_t> &right);
bool operator!= (const std::vector<hyrise::layouter::subset_t> &left, const std::vector<hyrise::layouter::subset_t> &right);

bool subset_t_lt(const hyrise::layouter::subset_t &left, const hyrise::layouter::subset_t &right);
bool sort_subset_by_size(const hyrise::layouter::subset_t &left, const hyrise::layouter::subset_t &right);

bool subset_t_content_equal(const hyrise::layouter::subset_t& left, const hyrise::layouter::subset_t& right);

int GCD(int a, int b);
int LCM(int a, int b);

template <class InputIterator, class T>
T sum(InputIterator first, InputIterator last, const T &value) {
  T ret = 0;

  while (first != last) if (*first++ == value) {
      ++ret;
    }

  return ret;
}

