// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <pthread.h>
#include <sys/time.h>
#include <queue>
#include <assert.h>


template<class T, void(T::*mem_fn)(void *)>
void *thunk(void *p) {
  std::pair<void *, void *> *a = static_cast<std::pair<void *, void *>* >(p);
  (static_cast<T *>(a->first)->*mem_fn)(a->second);
  delete a;
  return 0;
}

template <typename T>
class ParallelSort {
  typedef struct {
    std::vector<T> *data;
    size_t begin;
    size_t end;
  } sort_arg_t;

  typedef struct {
    std::vector<T> *data;
    size_t begin;
    size_t end;
    bool delete_data_when_done;
  } merge_arg_t;

  class merge_data_t {
   public:
    T value;
    size_t slice;
    size_t pos;
    friend bool operator<(const merge_data_t &a, const merge_data_t &b) {
      return a.value >= b.value;
    }
  };

  std::queue<merge_arg_t> pending_parts;
  unsigned remaining_parts;

  pthread_mutex_t parts_mutex;
  pthread_mutex_t parts_exist_mutex;

  std::vector<T> *data;
  size_t thread_count;

  void sort_thread(void *arg) {
    std::sort(((sort_arg_t *)arg)->data->begin() + ((sort_arg_t *)arg)->begin, ((sort_arg_t *)arg)->data->begin() + ((sort_arg_t *)arg)->end);
  }

  void merge_thread(void *arg) {
    merge_arg_t a, b, c;
    bool cont, quit;

    while (true) {
      cont = false;
      quit = false;

      size_t waiting = 0, remaining = 0;

      while (true) {
        pthread_mutex_lock(&parts_mutex);
        waiting = pending_parts.size();
        remaining = remaining_parts;
        pthread_mutex_unlock(&parts_mutex);

        if (waiting >= 2) {
          pthread_mutex_lock(&parts_mutex);
          a = pending_parts.front();
          pending_parts.pop();
          b = pending_parts.front();
          pending_parts.pop();
          remaining_parts -= 1;
          pthread_mutex_unlock(&parts_mutex);
          break;
        } else if (waiting + remaining >= 2) {
          if (pthread_mutex_trylock(&parts_exist_mutex) != 0) {
            return;
          }
        } else {
          return;
        }
      }

      c.data = merge_sorted(a, b);
      c.begin = 0;
      c.end = c.data->size();
      c.delete_data_when_done = true;

      pthread_mutex_lock(&parts_mutex);
      pending_parts.push(c);

      if (pending_parts.size() <= 2) {
        pthread_mutex_trylock(&parts_exist_mutex);
        pthread_mutex_unlock(&parts_exist_mutex);
      }

      pthread_mutex_unlock(&parts_mutex);
    }
  }

  static std::vector<T> *merge_sorted(merge_arg_t left, merge_arg_t right) {
    std::vector<T> *out = new std::vector<T>();
    size_t i_left = left.begin, i_right = right.begin;

    while (i_left < left.end && i_right < right.end) {
      if ((*(left.data))[i_left] < (*(right.data))[i_right]) {
        out->push_back((*(left.data))[i_left++]);
      } else {
        out->push_back((*(right.data))[i_right++]);
      }
    }

    while (i_left < left.end) {
      out->push_back((*(left.data))[i_left++]);
    }

    while (i_right < right.end) {
      out->push_back((*(right.data))[i_right++]);
    }

    // assert((left.end - left.begin) + (right.end - right.begin) == out->size());
    if (left.delete_data_when_done) {
      delete left.data;
    }

    if (right.delete_data_when_done) {
      delete right.data;
    }

    return out;
  }

 public:

  ParallelSort(std::vector<T> *_data, size_t _thread_count) : data(_data), thread_count(_thread_count) {
  }

  void sort() {
    sort_arg_t *thread_data;

    //          struct timeval t1, t2;
    //          gettimeofday(&t1, nullptr);

    if (thread_count == 1) {
      std::sort(data->begin(), data->end());
    }
    // divide input data on threads for sorting
    else {
      pthread_t *threads = new pthread_t[thread_count];
      thread_data = new sort_arg_t[thread_count];

      size_t begin = 0;
      size_t per_thread = data->size() / thread_count;
      size_t remainder = data->size() - per_thread * thread_count;

      for (int i = 0; i < thread_count; i++) {
        size_t size = per_thread + (i < remainder ? 1 : 0);
        thread_data[i].data = data;
        thread_data[i].begin = begin;
        thread_data[i].end = begin + size;
        begin += size;
      }

      for (int i = 0; i < thread_count; i++) {
        pthread_create(&threads[i], nullptr, thunk<ParallelSort, &ParallelSort::sort_thread>, new std::pair<void *, void *>(this, &thread_data[i]));
      }

      for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], nullptr);
      }

      delete threads;
    }

    //          gettimeofday(&t2, nullptr);
    //          printf("%f,", t2.tv_sec + (double)t2.tv_usec / 1000000 - t1.tv_sec - (double)t1.tv_usec / 1000000);
    //          gettimeofday(&t1, nullptr);

    if (thread_count == 1) {

    }
    // uses a single 2-way merge to merge the two sorted parts
    else if (thread_count == 2) {
      merge_arg_t a, b;
      a.data = thread_data[0].data;
      a.begin = thread_data[0].begin;
      a.end = thread_data[0].end;
      a.delete_data_when_done = false;

      b.data = thread_data[1].data;
      b.begin = thread_data[1].begin;
      b.end = thread_data[1].end;
      b.delete_data_when_done = false;

      std::vector<T> *out = ParallelSort::merge_sorted(a, b);
      data->swap(*out);
      delete out;
    }

    // uses a single n-way merge to merge the sorted parts.
    // this is about 2x slower than the solution below
    //          else {
    //                  std::vector<T> *out = new std::vector<T>();
    //                  merge_data_t d;
    //
    //                  std::priority_queue<merge_data_t> heap;
    //
    //                  for (int i = 0; i < thread_count; i++) {
    //                          d.pos = thread_data[i].begin;
    //                          d.value = (*(thread_data[i].data))[d.pos];
    //                          d.slice = i;
    //                          heap.push(d);
    //                  }
    //
    //                  while (heap.size() > 0) {
    //                          d = heap.top();
    //                          heap.pop();
    //
    //                          out->push_back(d.value);
    //
    //                          d.pos++;
    //                          if (d.pos < thread_data[d.slice].end) {
    //                                  d.value = (*(thread_data[d.slice].data))[d.pos];
    //                                  heap.push(d);
    //                          }
    //                  }
    //                  data->swap(*out);
    //                  delete out;
    //          }

    // uses threaded 2-way merges to merge pairs of sorted parts until
    // only one big sorted part is left
    else {
      pthread_mutex_init(&parts_mutex, nullptr);
      pthread_mutex_init(&parts_exist_mutex, nullptr);

      size_t merge_thread_count = thread_count / 2;

      if (merge_thread_count < 2) {
        merge_thread_count = 2;
      }

      remaining_parts = 0;

      for (int i = 0; i < thread_count; i++) {
        merge_arg_t a;
        a.data = thread_data[i].data;
        a.begin = thread_data[i].begin;
        a.end = thread_data[i].end;
        a.delete_data_when_done = false;
        pending_parts.push(a);
        remaining_parts++;
      }

      pthread_mutex_lock(&parts_exist_mutex);

      pthread_t *threads = new pthread_t[merge_thread_count];

      for (int i = 0; i < merge_thread_count; i++) {
        pthread_create(&threads[i], nullptr, thunk<ParallelSort, &ParallelSort::merge_thread>, new std::pair<void *, void *>(this, nullptr));
      }

      for (int i = 0; i < merge_thread_count; i++) {
        pthread_join(threads[i], nullptr);
      }

      delete threads;
      pthread_mutex_destroy(&parts_mutex);
      pthread_mutex_destroy(&parts_exist_mutex);

      merge_arg_t res = pending_parts.front();
      data->swap(*res.data);
      delete res.data;
    }

    //          gettimeofday(&t2, nullptr);
    //          printf("%f\n", t2.tv_sec + (double)t2.tv_usec / 1000000 - t1.tv_sec - (double)t1.tv_usec / 1000000);

    if (thread_count > 1) {
      delete thread_data;
    }
  }

  static void sort(std::vector<T> *data, size_t thread_count) {
    ParallelSort s(data, thread_count);
    s.sort();
  }
};

