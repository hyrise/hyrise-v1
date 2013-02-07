// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <iostream>

#include <boost/foreach.hpp>
#include "layout_utils.h"


void layouter::print(std::vector<std::vector<unsigned> >l) {
  std::cout << "[";
  BOOST_FOREACH(std::vector<unsigned> x, l) {
    BOOST_FOREACH(unsigned e, x) {
      std::cout << e << " ";
    }
    std::cout << "|";
  }
  std::cout << "]" << std::endl;;
}

void layouter::print(std::vector<unsigned> x) {
  BOOST_FOREACH(unsigned e, x) {
    std::cout << e << " ";
  }
  std::cout << std::endl;
}

void layouter::print(std::set<unsigned> x) {
  BOOST_FOREACH(unsigned e, x) {
    std::cout << e << " ";
  }
  std::cout << std::endl;
}

void layouter::print(int *x, int s) {
  for (int i = 0; i < s; ++i) {
    std::cout << x[i] << " ";
  }
  std::cout << std::endl;
}

void layouter::print(std::vector<double> d) {
  BOOST_FOREACH(double e, d) {
    std::cout << e << " ";
  }
  std::cout << std::endl;
}

std::vector<layouter::subset_t> &operator += (std::vector<layouter::subset_t> &a, const std::vector<layouter::subset_t> &b) {
  BOOST_FOREACH(layouter::subset_t i, b) {
    a.push_back(i);
  }
  return a;
}

std::vector<layouter::subset_t>   operator + (const std::vector<layouter::subset_t> &a, const std::vector<layouter::subset_t> &b) {
  std::vector<layouter::subset_t> result;
  BOOST_FOREACH(layouter::subset_t i, a) {
    result.push_back(i);
  }
  BOOST_FOREACH(layouter::subset_t i, b) {
    result.push_back(i);
  }
  return result;
}

bool operator== (const std::vector<layouter::subset_t> &left, const std::vector<layouter::subset_t> &right) {
  if (left.size() != right.size())
    return false;

  for (size_t i = 0; i < left.size(); ++i) {
    if (left[i] != right[i])
      return false;
  }
  return true;
}

bool operator!= (const std::vector<layouter::subset_t> &left, const std::vector<layouter::subset_t> &right) {
  return !(left == right);
}

bool sort_subset_by_size(const layouter::subset_t &left, const layouter::subset_t &right) {
  return left.size() > right.size();
}

bool subset_t_lt(const layouter::subset_t &left, const layouter::subset_t &right) {
  if (left.size() < right.size())
    return true;

  if (left.size() == right.size()) {
    // Now we need to compare the both vectors internally
    for (size_t i = 0; i < left.size(); ++i) {
      if (left[i] < right[i])
        return true;

      if (left[i] > right[i])
        return false;
    }
  }

  return false;
}

bool subset_t_content_equal(const layouter::subset_t& left, const layouter::subset_t& right)
{
    std::set<unsigned> left_set(left.begin(), left.end());
    std::set<unsigned> right_set(right.begin(), right.end());
    return left_set == right_set;
}

int GCD(int a, int b) {
  int s;

  if (a > b) {
    s = b;
  } else {
    s = a;
  }

  for (int i = s; i > 0; i--) {
    if ((a % i == 0) && (b % i == 0)) {
      return i;
    }
  }

  return -1;
}


int LCM(int a, int b) {
  return (a * b) / GCD(a, b);
}
