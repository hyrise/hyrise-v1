// Copyright (c) 2014 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include <thread>
#include <vector>

#include "helper/SparseVector.h"
#include "helper/vector_helpers.h"


class SparseTest : public ::testing::Test {
protected:
  size_t num = 10 * 1000 * 1000;
    
  using vector_type = hyrise::helper::SparseVector<long>;
  vector_type data;

  std::vector<long> data_old;

  std::string dbg(size_t num, size_t allocated, size_t empty) {
    std::stringstream s;
    s << "SparseVector: " << &data << std::endl;
    s << "Per Block: " << vector_type::per_mapping_size << std::endl;
    s << "Size: " << num << std::endl;
    s << "Allocated Blocks: " <<  allocated << "/13" << std::endl;
    s << "Empty Blocks: " << empty << "/13" << std::endl;
    return s.str();
  }

public:

  void SetUp() {
    data = vector_type(num, 1);
    data_old = std::vector<long>(num, 1);
  }
};


TEST_F(SparseTest, initialize) {
  ASSERT_EQ(num, data.size());
  ASSERT_EQ(1, data.get(0));
}

TEST_F(SparseTest, get_data) {
  ASSERT_EQ(num, data.size());
  ASSERT_EQ(1, data[0]);

  data[0] = 99;
  ASSERT_EQ(99, data.get(0));
}

TEST_F(SparseTest, alloc_one) {
  data.set(0, 1);
  ASSERT_EQ(1, data.get(0)); 
}

TEST_F(SparseTest, alloc_multiple) {
  auto tmp = vector_type::per_mapping_size;

  data.set(0, 99);
  data.set(1, 21);
  data.set(tmp, 192);

  ASSERT_EQ(99, data.get(0));
  ASSERT_EQ(21, data.get(1));
  ASSERT_EQ(192, data.get(tmp));
  ASSERT_EQ(num, data.size());
}

TEST_F(SparseTest, copy_ctor) {
  data.set(0, 1923);
  auto other = data;
  other.set(0, 99);

  ASSERT_EQ(1923, data.get(0));
  ASSERT_EQ(99, other.get(0));
}


TEST_F(SparseTest, push_emplace) {
  ASSERT_EQ(num, data.size());
  data.push_back(19923);
  data.emplace_back(2934);

  ASSERT_EQ(num + 2, data.size());
  ASSERT_EQ(19923, data.get(num));
  ASSERT_EQ(2934, data.get(num + 1));
}

TEST_F(SparseTest, concurrent_access) {
  hyrise::helper::SparseVector<long, 1024> other(num);
  auto offset = vector_type::per_mapping_size;
  auto iter = num / offset;

  size_t numThreads = 10;
  std::vector<std::thread> threads;
  for(size_t i=0; i < numThreads; ++i) {
    threads.emplace_back([&other, i, iter, offset](){
        // Allow Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        for(size_t j=0; j < iter; ++j) {
          other.set(j * offset + i, i);
        }
      });
  }

  for(auto& x : threads) {
    x.join();
  }

  for(size_t i=0; i < iter; ++i) {
    auto base = i * offset;
    for(size_t t=0; t < numThreads; ++t) {
      ASSERT_EQ(t, other.get(base + t));
    }
  }

}

TEST_F(SparseTest, debug) {
  ASSERT_EQ(dbg(num, 0, 13) , data.debug());
  data.set(0, 99);
  ASSERT_EQ(dbg(num, 1, 12) , data.debug());
  data.push_back(12);
  ASSERT_EQ(dbg(num + 1, 1, 12) , data.debug());
}

TEST_F(SparseTest, resize) {
  ASSERT_EQ(num, data.size());
  data.resize(num + 1, 99);
  ASSERT_EQ(99, data.get(num));
  data.resize(num + 2);
  ASSERT_EQ(1, data.get(num + 1));

  ASSERT_ANY_THROW(data.resize(0));
}

TEST_F(SparseTest, iter) {
  for(const auto& x : data) {
    ASSERT_EQ(1, x);
  }

  for(auto x=data.begin(), e=data.end(); x != e; x++) {
    ASSERT_EQ(1, *x);
  }

  for(auto x=data.cbegin(), e=data.cend(); x != e; ++x) {
    ASSERT_EQ(1, *x);
  }

}

TEST_F(SparseTest, push_and_set ) {
  data.push_back(99);
  data.set(num, 100);
  ASSERT_EQ(100, data.get(num));
}

TEST_F(SparseTest, cmpxchg) {
  data.push_back(99);
  data.set(num, 100);

  // Somehwere
  ASSERT_TRUE(data.cmpxchg(0, 1, 99));
  ASSERT_FALSE(data.cmpxchg(1, 22, 1));


  // End
  ASSERT_TRUE(data.cmpxchg(num, 100, 99));
  ASSERT_FALSE(data.cmpxchg(num, 100, 99));
}
