// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <assert.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <vector>

namespace hyrise {
namespace layouter {

struct adj_t {
  int *xadj;
  int *adjncy;
  int *adjwgt;
};

/*
 * This is a simple implementation for a non-sparse matrix data type
 * with easy accessors
 *
 */
template<typename T>
class Matrix {

 public:

  typedef T value_type;
  typedef Matrix<T> matrix_t;

  /*
   * Initialize the matrix with a given number of rows and columns
   * and a default value. In case no default value is given, the
   * default constructor of the default value is used to construct
   * the value
   */
  explicit Matrix(size_t columns);

  ~Matrix();

  /*
   * Skalar Operators
   */
  T get(size_t row, size_t col) const;
  Matrix<T> &set(size_t row, size_t col, T val);

  // Graph Ops
  // Number of columns
  int numEdges() const;
  // Number of coordinates where the value is > 0
  int numVertices() const {
    return _numCols;
  }


  adj_t buildAdjacency() const;

  // Helper Ops
  void print() const;

 private:

  int _numRows;
  int _numCols;
  std::vector<value_type> _data;
};

template<typename T>
int Matrix<T>::numEdges() const {
  size_t result = 0;

  // Find all symmetric entries where the count is larger than 0
  for (int i = 0; i < _numRows; ++i)
    for (int j = i; j < _numCols; ++j)
      if (get(i, j) > 0)
        ++result;

  return result;
}

template<typename T>
adj_t Matrix<T>::buildAdjacency() const {
  adj_t result;
  result.xadj = (int *) malloc((numVertices() + 1) * sizeof(*result.xadj));
  result.adjncy = (int *) malloc(2 * numEdges() * sizeof(*result.adjncy));
  result.adjwgt = (int *) malloc(2 * numEdges() * sizeof(*result.adjwgt));

  // Now write the output, for each vertice we have to write all edges
  int currentAdjxPos = 0;
  int currentAdjxOffset = 0;

  int currentAdjncyOffset = 0;
  for (int i = 0; i < numVertices(); ++i) {
    // For each vertex check all possible other vertices for connections
    for (int j = 0; j < _numCols; ++j) {
      if (i == j)
        continue;

      int value = get(i, j);
      if (value > 0) {
        result.adjncy[currentAdjncyOffset] = j;
        result.adjwgt[currentAdjncyOffset++] = value;
      }
    }

    result.xadj[currentAdjxPos++] = currentAdjxOffset;
    currentAdjxOffset = currentAdjncyOffset;
  }
  result.xadj[currentAdjxPos] = currentAdjncyOffset;
  return result;
}

template<typename T>
Matrix<T>::Matrix(size_t columns): _numRows((int)columns), _numCols((int)columns) {
  _data = std::vector<value_type>(_numCols * _numRows, 0);
}

template<typename T>
Matrix<T>::~Matrix() {
}

template<typename T>
T Matrix<T>::get(size_t row, size_t col) const {
  return _data[row * _numCols + col];
}

template<typename T>
Matrix<T> &Matrix<T>::set(size_t row, size_t col, T val) {
  _data[row * _numCols + col] = val;
  return *this;
}


template<typename T>
void Matrix<T>::print() const {
  std::stringstream buffer;

  buffer << "\t";

  //prepare header
  for (int i = 1; i <= _numCols ; ++i)
    buffer << "| " << i << "\t";
  buffer << std::endl;


  // Now each row
  for (int i = 0; i < _numRows ; ++i) {
    buffer << i + 1 << "\t";
    for (int j = 0; j < _numCols ; ++j) {
      if (i == j)
        buffer << "| " << "-" << "\t";
      else
        buffer << "| " << get(i, j) << "\t";
    }
    buffer << std::endl;
  }

  std::cout << buffer.str() << std::endl;
}

} } // namespace hyrise::layouter

