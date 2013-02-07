// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#ifndef SRC_LIB_STORAGE_CSB_TREE_H_
#define SRC_LIB_STORAGE_CSB_TREE_H_

#include <vector>
#include <iostream>
#include <list>
#include <memory>
#include <string.h>
#include <assert.h>
#include <boost/iterator/iterator_facade.hpp>

//#define DEBUG 1

#define CONVERT_LEAF(id) (IsLeaf(id.node) ? id.node : id.node->first_child + id.offset)
#define CUT(x) ((x) % 2 == 0 ? (x) / 2 : (x) / 2 + 1)
#define IsLeaf(x) !(((CSBLeafNodeUI*)x)->flag)

// DEBUG function
#ifdef DEBUG

#include <libgen.h>

#define DEBUG_CTL  basename((char*) __FILE__) << ":" << __LINE__ << " "
#define DEBUG_FUN()  std::cout <<  DEBUG_CTL << ">> " << __FUNCTION__ << "()" <<std::endl;
#define DEBUG_MSG(x) std::cout << DEBUG_CTL << x << std::endl;
#define DEBUG_ADD(x) std::cout << DEBUG_CTL <<std::hex << x << std::dec << std::endl;
#define DEBUG_PRINT(x) x->print()

#else

#define DEBUG_FUN()
#define DEBUG_MSG(x)
#define DEBUG_ADD(x)
#define DEBUG_PRINT(x)

#endif



namespace hyrise {
namespace index {

// Cache Line Size
const unsigned cache_line_size = 64;
const unsigned pointer_size = sizeof(void *);
const unsigned key_size = sizeof(unsigned int);


// this is calculatable but a litte harder
// for 64 bit systems since the 4 byte key is addressed
// by an 8 byte pointer
const unsigned number_possible_keys = 12;
const unsigned number_leaf_values = 4;
const unsigned initial_node_group_size = 2;


struct CSBLeafNodeUI;

// Internal Node
struct CSBNodeUI {

#ifdef DEBUG
  static size_t ___num_alloc;
  static size_t ___num_free;
#endif



  typedef unsigned int key_type;

  // Number of keys used
  unsigned int num_keys;
  // Pointer to the first child
  CSBNodeUI *first_child;
  // Key List 64 - num_keys - first_child
  unsigned int keys[number_possible_keys];

  explicit CSBNodeUI(bool with_memory = true);

  ~CSBNodeUI();

  void init();

  void clean() {
    num_keys = 0;
    first_child = 0;
  }

  // Add Node Child
  void addChild(unsigned index, CSBNodeUI *e) {
    memcpy(first_child + index, e, cache_line_size);
  }


  // Add Leaf Node child, alias for set child
  void addChild(unsigned index, CSBLeafNodeUI *e);

  // Set the child value
  void setChild(unsigned index, CSBLeafNodeUI *e);

  // Set the child value
  void setChild(unsigned index, CSBNodeUI *e);

  void push_back(CSBLeafNodeUI *e);

  // Insert the child at a given position
  void insert_for_key(key_type k, CSBLeafNodeUI *e);


  void push_back(CSBNodeUI *e);

  // Return the pointer to the indexed child
  CSBNodeUI *lookup(unsigned index) const {
    return first_child + index;

  }


  // Get the next child
  CSBNodeUI *getNext() {
    return first_child + num_keys;
  }

  void addKey(unsigned k) {
    keys[num_keys++] = k;
  }

  void print() const;

};


struct CSBPairUI {
  unsigned int key;
  unsigned int value;

  CSBPairUI(unsigned int k, unsigned int v): key(k), value(v)
  {}

  CSBPairUI(): key(0), value(0)
  {}

  CSBPairUI(const CSBPairUI &c) {
    key = c.key;
    value = c.value;
  }
};


// Leaf Node
struct CSBLeafNodeUI {

  typedef unsigned int key_type;

  // 4
  unsigned int num_keys;

  // set to null always, 8
  void *flag;

  // Forward and backward pointer ( 8+8 )
  CSBLeafNodeUI *prev;
  CSBLeafNodeUI *next;

  // Pairs
  CSBPairUI entries[4];

  CSBLeafNodeUI(): num_keys(0), flag(0), prev(0), next(0) {
  }


  void clean() {
    num_keys = 0;
    flag = 0;
    prev = 0;
    next = 0;
  }

  void print() const {
    std::cout << "<LeafNode(" << this << "):";

    for (unsigned i = 0; i < num_keys; ++i) {
      std::cout << "[" << entries[i].key << ":" << entries[i].value << "]";
    }

    std::cout << ">" << std::endl;
  }
};


/*
  Iterator for the CSBTree

  uses the basics provided by boost::iterator
*/
class CSBTreeIterator
    : public boost::iterator_facade<CSBTreeIterator, CSBPairUI, boost::forward_traversal_tag> {

 public:
  CSBTreeIterator(): _leaf(0), _num_key(0)
  {}

  explicit CSBTreeIterator(CSBLeafNodeUI *l,  CSBLeafNodeUI::key_type nk = 0): _leaf(l), _num_key(nk) {
  }

  virtual ~CSBTreeIterator() {
  }

  // Since the basics start with a given leaf node in the tree,
  // we follow this until we reach a given point
  void increment() {
    if (_num_key + 1 < _leaf->num_keys) {
      ++_num_key;

    } else {

      _num_key = 0;

      if (_leaf->next && _leaf->next->num_keys > 0) {
        _leaf = _leaf->next;
      } else {
        _leaf = 0;
      }
    }
  }

  // This is the forward iterator, and we can define
  // equal based on the pointer and the position inside
  // the leaf node
  bool equal(CSBTreeIterator const &other) const {
    return this->_leaf == other._leaf && this->_num_key == other._num_key;
  }

  // To derefence we return the entry from the leaf node
  CSBPairUI &dereference() const {
    return _leaf->entries[_num_key];
  }

 private:

  CSBLeafNodeUI *_leaf;
  CSBLeafNodeUI::key_type _num_key;
};


/*
  This class is an implementation of the CSB+ Tree presented by Rao et al.
  http://portal.acm.org/citation.cfm?id=335449
*/
class CSBTree {

  CSBNodeUI *root;

  // Stores the number of values
  size_t _num_values;

  // This is used to calculate the memory consumption
  size_t _num_leafs;
  size_t _num_internals;

 public:

  typedef unsigned int value_type;
  typedef unsigned int key_type;

  typedef unsigned int *value_ptr;
  typedef unsigned int *key_ptr;

  typedef CSBPairUI pair_type;
  typedef CSBPairUI *pair_ptr;

  typedef CSBLeafNodeUI leaf_node;
  typedef CSBNodeUI internal_node;

  typedef CSBLeafNodeUI *leaf_node_ptr;
  typedef CSBNodeUI *internal_node_ptr;

  // This is required by the iterator protocol
  typedef CSBTreeIterator iterator;
  typedef CSBTreeIterator const_iterator;

  CSBTree();

  ~CSBTree();

  // Insert a new k,v
  void insert(key_type k, value_type v);

  // Find a certain k
  pair_ptr find(key_type k);

  // delete a key
  void del(key_type k);


  void print(std::ostream &w = std::cout);

  // Returns the number of stored values
  size_t num_values();

  // Returns the number of leafs
  inline size_t num_leafs() const {
    return _num_leafs;
  }

  // returns the number of internal nodes
  inline size_t num_internals() const {
    return _num_internals;
  }

  inline size_t memory_consumption() const {
#ifdef DEBUG
    std::cout << "Alloc vs Freed" << std::endl;
    std::cout << CSBNodeUI::___num_alloc << std::endl;
    std::cout << CSBNodeUI::___num_free << std::endl;
#endif

    return sizeof(CSBTree) + num_leafs() * cache_line_size + num_internals() * cache_line_size;
  }

  // returns an iterator pointing to the beginning of the tree
  inline iterator begin() {
    return iterator(_begin);
  }

  // returns an empty iterator that marks the end of the tree
  inline iterator end() {
    return iterator();
  }

  inline iterator at(key_type k) {
    leaf_node_ptr t = (leaf_node_ptr) CONVERT_LEAF(find_leaf(k));

    key_type insertion_point = 0;

    while (t->entries[insertion_point].key < k && insertion_point < t->num_keys) {
      ++insertion_point;
    }

    return iterator(t, insertion_point);
  }

 private:

  typedef struct _node_id {
    // parent node
    internal_node_ptr node;
    // the depth
    unsigned level;
    // the offset
    unsigned offset;

    // the path, if the path is set ok, if not ignore
    std::list<CSBNodeUI *> *path;

    ~_node_id() {
    }

  } _node_id;

  leaf_node_ptr _begin;


  // Typedef to capture the node list
  typedef std::list<CSBNodeUI *> _node_path;
  typedef std::list<CSBNodeUI *> *_node_path_ptr;

  _node_id find_leaf(key_type k, bool withPath = false);

  // Insert into the leaf and split the node before
  void insert_into_leaf_after_splitting(const _node_path_ptr path,
                                        leaf_node_ptr leaf,
                                        key_type k,
                                        value_type v);



  leaf_node_ptr split_leaf_node(leaf_node_ptr leaf,
                                key_type k,
                                value_type v);

  // Insert the leaf node into the parent after it was split
  void insert_into_parent_after_splitting(const _node_path_ptr path,
                                          internal_node_ptr first_child,
                                          key_type k);

  // Insert the internal node into the parent after it was split
  void insert_into_parent_after_splitting(const _node_path_ptr path,
                                          leaf_node_ptr first_child,
                                          key_type k);


  // Directly insert into the leaf node
  void insert_into_leaf(leaf_node_ptr leaf, key_type k, value_type v);

  void print_path(_node_path_ptr p) const {
    _node_path::iterator it;

    for (it = p->begin(); it != p->end(); it++) {
      (*it)->print();
    }
  }

};
}
}


#endif  // SRC_LIB_STORAGE_CSB_TREE_H_
