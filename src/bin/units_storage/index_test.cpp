// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "testing/test.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include <stdio.h>

#include <storage/csb_tree.h>

using namespace hyrise::index;

class IndexTests : public ::hyrise::Test {};

TEST_F(IndexTests, random_insert) {

  CSBTree idx;
  {
    idx.insert(999, 999);
    idx.insert(111, 111);
    idx.insert(444, 444);
    idx.insert(333, 333);
    idx.insert(222, 222);

    //idx.print(opt);

    CSBTree::iterator begin, end;

    begin = idx.begin();
    end = idx.end();

    ASSERT_EQ(111u, (*begin).key);
    ++begin;

    ASSERT_EQ(222u, (*begin).key);
    ++begin;

    ASSERT_EQ(333u, (*begin).key);
    ++begin;

    ASSERT_EQ(444u, (*begin).key);
    ++begin;

    ASSERT_EQ(999u, (*begin).key);
  }
}

TEST_F(IndexTests, random_insert_big) {
  CSBTree idx;
  {
    idx.insert(999, 999);
    idx.insert(111, 111);
    idx.insert(444, 444);
    idx.insert(333, 333);
    idx.insert(222, 222);
    idx.insert(888, 888);
    idx.insert(555, 555);
    idx.insert(777, 777);
    idx.insert(666, 666);
    idx.insert(10, 10);
    idx.insert(5, 5);

    //idx.print(opt);
    CSBTree::iterator begin, end;

    begin = idx.begin();
    end = idx.end();

    ASSERT_EQ(5u, (*begin).key);
    ++begin;

    ASSERT_EQ(10u, (*begin).key);
    ++begin;

    ASSERT_EQ(111u, (*begin).key);
    ++begin;

    ASSERT_EQ(222u, (*begin).key);
    ++begin;

    ASSERT_EQ(333u, (*begin).key);
    ++begin;

    ASSERT_EQ(444u, (*begin).key);
    ++begin;

    ASSERT_EQ(555u, (*begin).key);
    ++begin;

    ASSERT_EQ(666u, (*begin).key);
    ++begin;

    ASSERT_EQ(777u, (*begin).key);
    ++begin;

    ASSERT_EQ(888u, (*begin).key);
    ++begin;

    ASSERT_EQ(999u, (*begin).key);
  }

}

TEST_F(IndexTests, random_insert_medium) {
  CSBTree idx;

  idx.insert(999, 999);
  idx.insert(111, 111);
  idx.insert(444, 444);
  idx.insert(333, 333);
  idx.insert(222, 222);
  idx.insert(888, 888);
  idx.insert(555, 555);
  idx.insert(777, 777);
  idx.insert(666, 666);
  //idx.insert(10,10);
  //idx.insert(5,5);

  //idx.print(opt);
  CSBTree::iterator begin, end;

  begin = idx.begin();
  end = idx.end();

  ASSERT_EQ(111u, (*begin).key);
  ++begin;

  ASSERT_EQ(222u, (*begin).key);
  ++begin;

  ASSERT_EQ(333u, (*begin).key);
  ++begin;

  ASSERT_EQ(444u, (*begin).key);
  ++begin;

  ASSERT_EQ(555u, (*begin).key);
  ++begin;

  ASSERT_EQ(666u, (*begin).key);
  ++begin;

  ASSERT_EQ(777u, (*begin).key);
  ++begin;

  ASSERT_EQ(888u, (*begin).key);
  ++begin;

  ASSERT_EQ(999u, (*begin).key);

}

TEST_F(IndexTests, index_tree_node) {
  CSBNodeUI tn;
  ASSERT_EQ(64u, sizeof(tn));

  CSBLeafNodeUI l;
  ASSERT_EQ(64u, sizeof(l));

}

TEST_F(IndexTests, index_one_level_insert) {
  CSBTree idx;

  idx.insert(1, 1);
  idx.insert(0, 0);

  CSBTree::pair_ptr p = idx.find(0);
  ASSERT_TRUE(nullptr != p);
  ASSERT_EQ(0u, p->value);

  idx.insert(10, 10);
  p = idx.find(10);
  ASSERT_TRUE(nullptr != p);

  ASSERT_EQ(10u, p->value);
}

TEST_F(IndexTests, index_split_first_level) {
  CSBTree idx;

  idx.insert(1, 1);
  idx.insert(0, 0);

  CSBTree::pair_ptr p = idx.find(0);
  ASSERT_TRUE(nullptr != p);
  ASSERT_EQ(0u, p->value);

  idx.insert(10, 10);
  p = idx.find(10);
  ASSERT_TRUE(nullptr != p);
  ASSERT_EQ(10u, p->value);


  idx.insert(8, 8);
  idx.insert(9, 9);
  idx.insert(10, 10);

  p = idx.find(8);
  ASSERT_TRUE(nullptr != p);
  ASSERT_EQ(8u, p->value);

  p = idx.find(10);
  ASSERT_TRUE(nullptr != p);
  ASSERT_EQ(10u, p->value);
}

TEST_F(IndexTests, index_deep_test) {
  {
    CSBTree idx;

    unsigned upper = 161;
    CSBTree::pair_ptr tmp;

    for (unsigned i = 0; i <= upper; ++i) {
      idx.insert(i, i);
      tmp = idx.find(i);

      ASSERT_EQ(tmp->key, i);
      ASSERT_EQ(tmp->value, i);
    }

    CSBTree::pair_ptr p = idx.find(upper);
    ASSERT_TRUE(nullptr != p);
    ASSERT_EQ(upper, p->value);

    idx.memory_consumption();

    //idx.print();
  }

#ifdef DEBUG
  std::cout << CSBNodeUI::___num_free << std::endl;
#endif
}

TEST_F(IndexTests, index_statistics) {
  CSBTree idx;
  {
    unsigned upper = 1000;

    for (unsigned i = 0; i < upper; ++i) {
      idx.insert(i, i);
    }

    ASSERT_EQ(1000u, idx.num_values());
  }
}

TEST_F(IndexTests, index_mem_consumption) {
  CSBTree idx;

  ASSERT_EQ(sizeof(CSBTree), idx.memory_consumption());
  ASSERT_EQ(0u, idx.num_values());

  idx.insert(0, 0);
  ASSERT_EQ(sizeof(CSBTree) + 64, idx.memory_consumption());

  unsigned upper = 20;

  for (unsigned i = 1; i < upper; ++i) {
    idx.insert(i, i);
  }

  ASSERT_EQ(sizeof(CSBTree) + 8 * 64, idx.memory_consumption());
  ASSERT_EQ(20u, idx.num_values());
}

TEST_F(IndexTests, index_iterator_test) {
  CSBTree idx;

  srand(time(nullptr));

  unsigned upper = 20;
  unsigned j = upper - 1;

  for (unsigned i = 1; i < upper; ++i, --j) {
    idx.insert(i, j);
  }

  j = upper - 1;
  for (CSBTree::pair_type p: idx) {
    ASSERT_EQ(j--, p.value);
  }

}

TEST_F(IndexTests, index_find_smaller) {
  srand(time(nullptr));

  CSBTree idx;
  std::vector<unsigned> list;
  std::set<unsigned> set_list;

  std::vector<unsigned> list2;

  for (unsigned i = 0; i < 40; ++i) {
    unsigned tmp = rand() % 1000 + 1;
    set_list.insert(tmp);
    idx.insert(tmp, tmp);

    list2.push_back(tmp);
  }

  //idx.print(opt);

  // sort the vector
  list = std::vector<unsigned>(set_list.begin(), set_list.end());

  unsigned pos = rand() % list.size();

  CSBTree::iterator end = idx.at(list[pos]);
  CSBTree::iterator begin = idx.begin();


  //BOOST_TEST_MESSAGE( list[pos] );


  for (unsigned i = 0; begin != end; ++begin, ++i) {
    CSBPairUI v = *begin;
    //BOOST_TEST_MESSAGE ( v.key << " " << list[i] );
    ASSERT_EQ(v.key, list[i]);
  }

  std::stringstream s;
  for (unsigned i: list2) {
    s << i << " ";
  }

  //BOOST_TEST_MESSAGE( s.str() );

}

TEST_F(IndexTests, index_find_bigger) {
  srand(time(nullptr));

  CSBTree idx;
  std::vector<unsigned> list;
  std::set<unsigned> set_list;

  std::vector<unsigned> list2;

  for (unsigned i = 0; i < 40; ++i) {
    unsigned tmp = rand() % 1000 + 1;
    set_list.insert(tmp);
    idx.insert(tmp, tmp);

    list2.push_back(tmp);
  }


  // sort the vector
  list = std::vector<unsigned>(set_list.begin(), set_list.end());

  unsigned pos = rand() % list.size();

  CSBTree::iterator end = idx.end();
  CSBTree::iterator begin = idx.at(list[pos]);

  //idx.print(opt);

  //BOOST_TEST_MESSAGE(list[pos]);

  for (unsigned i = pos; begin != end; ++begin, ++i) {
    CSBPairUI v = *begin;
    //BOOST_TEST_MESSAGE ( v.key << " " << list[i] );
    ASSERT_EQ(v.key, list[i]);
  }

  std::stringstream s;
  for (unsigned i: list2) {
    s << i << " ";
  }

  //BOOST_TEST_MESSAGE( s.str() );

}

TEST_F(IndexTests, index_random_fail) {
  srand(time(nullptr));

  unsigned vals[40] = {55, 199, 898, 723, 377, 228, 897, 333, 532, 961, 971, 680, 373, 258, 675, 388, 477, 434, 336, 338, 498, 347, 527, 673, 756, 906, 545, 12, 141, 128, 98, 693, 82, 165, 61, 556, 38, 802, 92, 659};

  std::vector<unsigned> list;

  CSBTree idx;
  {
    for (unsigned i = 0; i < 40; ++i) {
      idx.insert(vals[i], vals[i]);
      list.push_back(vals[i]);
    }

    std::sort(list.begin(), list.end());
    unsigned pos = rand() % list.size();
    CSBTree::iterator end = idx.end();
    CSBTree::iterator begin = idx.at(list[pos]);

    for (unsigned i = pos; begin != end; ++begin, ++i) {
      CSBPairUI v = *begin;
      ASSERT_EQ(v.key, list[i]);
    }
  }
}

TEST_F(IndexTests, index_random_fail_2) {
  srand(time(nullptr));
  unsigned vals[40] = {717, 223, 520, 545, 977, 328, 610, 482, 357, 264, 320, 258, 287, 319, 639, 876, 173, 732, 84, 235, 80, 940, 202, 55, 597, 779, 968, 571, 608, 369, 26, 795, 902, 133, 350, 750, 346, 767, 174};

  std::vector<unsigned> list;

  CSBTree idx;

  for (unsigned i = 0; i < 40; ++i) {
    idx.insert(vals[i], vals[i]);
    list.push_back(vals[i]);
  }

  std::sort(list.begin(), list.end());
  unsigned pos = rand() % list.size();

  //idx.print(opt);
  //BOOST_TEST_MESSAGE(list[pos]);

  CSBTree::iterator end = idx.end();
  CSBTree::iterator begin = idx.at(list[pos]);

  for (unsigned i = pos; begin != end; ++begin, ++i) {
    CSBPairUI v = *begin;
    //BOOST_TEST_MESSAGE ( v.key << " " << list[i] );
    ASSERT_EQ(v.key, list[i]);
  }

}
TEST_F(IndexTests, index_fixed_pos) {
  srand(time(nullptr));
  unsigned vals[40] = {717, 223, 520, 545, 977, 328, 610, 482, 357, 264, 320, 258, 287, 319, 639, 876, 173, 732, 84, 235, 80, 940, 202, 55, 597, 779, 968, 571, 608, 369, 26, 795, 902, 133, 350, 750, 346, 767, 174};

  std::vector<unsigned> list;

  CSBTree idx;

  for (unsigned i = 0; i < 40; ++i) {
    idx.insert(vals[i], vals[i]);
    list.push_back(vals[i]);
  }

  std::sort(list.begin(), list.end());
  //idx.print(opt);
  CSBTree::iterator end = idx.end();
  CSBTree::iterator begin = idx.at(369);

  for (unsigned i = (std::find(list.begin(), list.end(), 369) - list.begin()); begin != end; ++begin, ++i) {
    CSBPairUI v = *begin;
    //BOOST_TEST_MESSAGE ( v.key << " " << list[i] );
    ASSERT_EQ(v.key, list[i]);
  }

}

TEST_F(IndexTests, index_fixed_pos_2) {
  srand(time(nullptr));
  unsigned vals[40] = {575, 704, 828, 277, 345, 570, 693, 980, 85, 731, 139, 832, 755, 36, 953, 988, 938, 347, 151, 32, 592, 933, 835, 604, 211, 11, 214, 357, 738, 655, 305, 58, 73, 300, 246, 363, 501, 504, 668, 393};

  std::vector<unsigned> list;

  CSBTree idx;

  for (unsigned i = 0; i < 40; ++i) {
    idx.insert(vals[i], vals[i]);
    list.push_back(vals[i]);
  }

  std::sort(list.begin(), list.end());
  //idx.print(opt);

  CSBTree::iterator end = idx.end();
  CSBTree::iterator begin = idx.at(570);

  for (unsigned i = (std::find(list.begin(), list.end(), 570) - list.begin()); begin != end; ++begin, ++i) {
    CSBPairUI v = *begin;
    //BOOST_TEST_MESSAGE ( v.key << " " << list[i] );
    ASSERT_EQ(v.key, list[i]);
  }

}

TEST_F(IndexTests, index_fixed_pos_3) {
  srand(time(nullptr));
  unsigned vals[40] = {516, 987, 802, 913, 699, 956, 485, 362, 214, 201, 129, 366, 691, 301, 87, 104, 891, 390, 341, 166, 105, 70, 997, 931, 310, 39, 84, 337, 881, 705, 488, 257, 250, 953, 152, 264, 50, 537, 332, 29};

  std::vector<unsigned> list;

  CSBTree idx;

  for (unsigned i = 0; i < 40; ++i) {
    idx.insert(vals[i], vals[i]);
    list.push_back(vals[i]);
  }

  std::sort(list.begin(), list.end());

  CSBTree::iterator end = idx.end();
  CSBTree::iterator begin = idx.begin();

  //idx.print();

  unsigned i = 0;

  for (i = 0; begin != end; ++begin, ++i) {
    CSBPairUI v = *begin;
    //BOOST_TEST_MESSAGE ( v.key << " " << list[i] );
    ASSERT_EQ(v.key, list[i]);
  }

  ASSERT_EQ(40u, i);
}
