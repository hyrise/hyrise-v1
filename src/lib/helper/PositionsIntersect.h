// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <algorithm>
#include <iterator>
// produces sorted intersection of two pos lists. based on
// baeza-yates algorithm with average complexity nicely
// adapting to the smaller list size, whereas std::set_intersection
// iterates over both lists with linear complexity.
template <typename IterT, typename OutputIter>
void intersect_pos_list(IterT beg1, IterT end1, IterT beg2, IterT end2, OutputIter resultIter, bool first_sorted=true, bool second_sorted=true)
{
  std::vector<typename std::iterator_traits<IterT>::value_type> input1_sorted, input2_sorted;

  auto size_1 = std::distance(beg1, end1);
  auto size_2 = std::distance(beg2, end2);

  if (!first_sorted) {
    // copy input 1 and sort it
    input1_sorted.reserve(size_1);
    input1_sorted.insert(input1_sorted.end(), beg1, end1);
    std::sort(input1_sorted.begin(), input1_sorted.end());
    beg1 = input1_sorted.begin();
    end1 = input1_sorted.end();
  }
  if (!second_sorted) {
    // copy input 2 and sort it
    input2_sorted.reserve(size_2);
    input2_sorted.insert(input2_sorted.end(), beg2, end2);
    std::sort(input2_sorted.begin(), input2_sorted.end());
    beg2 = input2_sorted.begin();
    end2 = input2_sorted.end();
  }

  // if one of the inputs is empty,
  // return as intersect is empty
  if ((size_1 <= 0) or (size_2 <= 0)) return;

  // if input 1 and input 2 do not overlap at all,
  // return as intersect is empty
  if ((*(end1 - 1) < *beg2) or (*(end2 - 1) < *beg1)) return;

  // if both lists are very small,
  // use std intersection as iterating is faster than binary search
  if ((size_1+size_2) < 20) {
    std::set_intersection(beg1, end1, beg2, end2, resultIter);
    return;
  }

  // make sure input 1 is larger than input 2
  if (size_1 < size_2) {
    std::swap(end1, end2);
    std::swap(beg1, beg2);
    std::swap(size_1, size_2);
  }

  // find overlap by searching in smaller input (input 2)
  beg2 = std::lower_bound(beg2, end2, *beg1);
  end2 = std::upper_bound(beg2, end2, *(end1 - 1));
  size_2 = std::distance(beg2, end2);

  // search median of input 2 in input 1
  // effectively dividing larger input in two
  auto m = beg2 + (size_2 / 2);
  auto m_in_1 = std::lower_bound(beg1, end1, *m);

  // and recursively do the rest
  if (*m_in_1 == *m) {
    intersect_pos_list(beg1, m_in_1, beg2, m, resultIter);
    *resultIter++ = *m; // add m to result iterator
    intersect_pos_list(m_in_1+1, end1, m+1, end2, resultIter);
  } else {
    intersect_pos_list(beg1, m_in_1, beg2, m, resultIter);
    intersect_pos_list(m_in_1, end1, m+1, end2, resultIter);
  }
}

