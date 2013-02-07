#include "csb_tree.h"
#include <assert.h>
#include <iostream>

#ifndef nullptr
#define nullptr 0
#endif


using namespace hyrise::index;

#ifdef DEBUG
size_t CSBNodeUI::___num_free = 0;
size_t CSBNodeUI::___num_alloc = 0;
#endif


void CSBNodeUI::print() const {
  std::cout << "<InternalNode: ";

  for (unsigned i = 0; i < num_keys; ++i) {
    std::cout << keys[i] << " ";
  }

  if (IsLeaf(lookup((unsigned) 0))) {
    std::cout << "{";

    for (unsigned i = 0; i <= num_keys; ++i) {
      ((CSBLeafNodeUI *) lookup(i))->print();
    }

    std::cout << "}";
  } else {

    std::cout << "{";

    for (unsigned i = 0; i <= num_keys; ++i) {
      lookup(i)->print();

    }

    std::cout << "}";
  }


  std::cout << ">" << std::endl;
}


void ___free(CSBNodeUI *p) {
  DEBUG_FUN();

  // Ignore if empty
  if (p->first_child == nullptr) {
    return;
  }

  if (IsLeaf(p)) {
    return;
  }

  // We are on the bottom level
  if (IsLeaf(p->first_child)) {
#ifdef DEBUG
    ++CSBNodeUI::___num_free;
#endif
    //free(p->first_child);
    return;
  }

  // Free all lower levels
  for (unsigned i = 0; i <= p->num_keys; ++i) {
    ___free(p->first_child + i);
  }

#ifdef DEBUG
  ++CSBNodeUI::___num_free;
#endif
  //free(p->first_child);
}

CSBNodeUI::~CSBNodeUI() {
  DEBUG_MSG("~CSBNodeUI");
  DEBUG_MSG(this);

  ___free(this);
}


void CSBNodeUI::addChild(unsigned index, CSBLeafNodeUI *e) {
  DEBUG_FUN();

  setChild(index, e);
}

// If the leafnode is inserted into the internal node
// we know that no more data will be allocated below thos level
void CSBNodeUI::setChild(unsigned index, CSBLeafNodeUI *e) {

  DEBUG_FUN();

  memcpy(first_child + index, e, cache_line_size);

  CSBLeafNodeUI *l, *r;

  if (index == 0) {
    DEBUG_MSG("index == 0");

    l = (CSBLeafNodeUI *)(first_child);
    r = (CSBLeafNodeUI *)(first_child + 1);

    l->next = r;
    r->prev = l;

    assert(l->next->entries[0].key == r->entries[0].key);

  } else if (index > 0 && index < num_keys) {
    DEBUG_MSG("index > 0 && index < num_keys");

    l = (CSBLeafNodeUI *)(first_child + index - 1);
    r = (CSBLeafNodeUI *)(first_child + index);

    l->next = r;
    r->prev = l;

    l = (CSBLeafNodeUI *)(first_child + index);
    r = (CSBLeafNodeUI *)(first_child + index + 1);

    l->next = r;
    r->prev = l;


    assert(l->entries[0].key != r->entries[0].key);

  } else if (index == num_keys) {
    DEBUG_MSG("index == num_keys");

    l = (CSBLeafNodeUI *)(first_child + index - 1);
    r = (CSBLeafNodeUI *)(first_child + index);

    l->next = r;
    r->prev = l;

    assert(l->next->entries[0].key == r->entries[0].key);
  }
}


void CSBNodeUI::push_back(CSBLeafNodeUI *e) {
  DEBUG_FUN();
  addChild(num_keys, e);
}

// When an internal node is set, we have to copy the
// children of the parameter node as well
void CSBNodeUI::setChild(unsigned index, CSBNodeUI *e) {
  assert(! IsLeaf(e));
  DEBUG_FUN();

  // Copy the value from the node but than explicitely copy
  // the structure
  memcpy(first_child + index, e, cache_line_size);
  (first_child + index)->first_child = e->first_child;

  e->first_child = nullptr;
}


void CSBNodeUI::push_back(CSBNodeUI *e) {
  DEBUG_FUN();

  // Copys the memory for the node
  setChild(num_keys, e);

  if (IsLeaf(lookup(num_keys)->first_child)) {
    CSBLeafNodeUI *l, *r;

    CSBNodeUI *tmp = lookup(num_keys - 1);
    l = (CSBLeafNodeUI *)(tmp->first_child + tmp->num_keys);

    tmp = lookup(num_keys);
    r = (CSBLeafNodeUI *) tmp->first_child;

    l->next = r;
    r->prev = l;
  }
}


void CSBNodeUI::insert_for_key(key_type k, CSBLeafNodeUI *e) {

  DEBUG_FUN();
  DEBUG_MSG(k);

  // 1. Find the insertion point for the key
  // 2. move all elements one to the right
  // 3. insert the child
  CSBLeafNodeUI *tmp = (CSBLeafNodeUI *) first_child;

  // Determines the insertion point for the keys
  key_type insertion_point = 0;

  while (keys[insertion_point] < k  && insertion_point < num_keys) {
    ++insertion_point;
  }

  DEBUG_MSG(insertion_point);
  DEBUG_MSG(num_keys);

  for (unsigned i = num_keys; i > insertion_point; --i) {
    keys[i] = keys[i - 1];
  }

  ++num_keys;

  if (insertion_point == 0) {
    keys[insertion_point] =  tmp->entries[0].key < k ? k : tmp->entries[0].key;
  } else {
    keys[insertion_point] =  k;
  }



  // Increment the number of used keys
  for (unsigned i = num_keys; i > insertion_point; --i) {
    setChild(i, (CSBLeafNodeUI *)(first_child + i - 1));
  }

  setChild(insertion_point + (tmp->entries[0].key < k ? 1 : 0), e);



}

CSBNodeUI::CSBNodeUI(bool with_memory): num_keys(0), first_child(0) {
  DEBUG_MSG("CSBNodeUI CTOR");
  DEBUG_MSG(this);
  // Create the correct memory region
  if (posix_memalign((void **) &first_child, cache_line_size, (number_possible_keys + 1) * cache_line_size) != 0) {
    throw std::bad_alloc();
  }
  memset(first_child, 0, (number_possible_keys + 1) * cache_line_size);

#ifdef DEBUG
  ++___num_alloc;
#endif
}


void CSBNodeUI::init() {
  num_keys = 0;
  first_child = 0;

  for (unsigned i = 0; i < number_possible_keys; ++i) {
    keys[i] = 0;
  }
}


// Constructor for the index
CSBTree::CSBTree(): root(0), _num_values(0), _num_leafs(0), _num_internals(0), _begin(0) {

#ifdef DEBUG
  CSBNodeUI::___num_alloc = 0;
  CSBNodeUI::___num_free = 0;
#endif
}

CSBTree::~CSBTree() {
  DEBUG_MSG("~CSBTree");

  if (!IsLeaf(root)) {
    delete root;
  } else {
    delete(CSBLeafNodeUI *) root;
  }

  // TODO fix leaking leafs
}


// This message is used to find an actual leaf node
// identified by the _node_id struct which contains a link
// to the parent, the level and the offset
CSBTree::_node_id CSBTree::find_leaf(CSBTree::key_type k, bool withPath) {
  internal_node_ptr u = root;
  internal_node_ptr parent = nullptr;
  unsigned i, level;


  // Store the path for all parent elements
  std::list<CSBNodeUI *> *path = nullptr;

  if (withPath) {
    path = new std::list<CSBNodeUI *>();
  }


  if (IsLeaf(root)) {
    _node_id r =  {root, 0, 0, path};
    return r;
  }

  level = 0;
  i = 0;
  while (!IsLeaf(u)) {
    // Choose the correct pointer in u and
    i = 0;

    while (i < u->num_keys) {
      if (k >= u->keys[i]) {
        ++i;
      } else {
        break;
      }
    }

    parent = u;

    // Add the path to the list
    if (withPath) {
      path->push_front(parent);
    }

    ++level;
    u = u->first_child + i;
  }

  _node_id result = {parent, level, i, path};

  return result;
}


// This is the public find method
CSBTree::pair_ptr CSBTree::find(CSBTree::key_type k) {
  unsigned i = 0;
  leaf_node_ptr ln;

  if (root == nullptr) {
    return nullptr;
  }

  if (!IsLeaf(root)) {

    // We found the leaf node that should contain k
    _node_id id = find_leaf(k);

    if (id.node == nullptr) {
      return nullptr;
    }

    ln = (leaf_node_ptr)(IsLeaf(id.node) ? id.node  : id.node->first_child + id.offset);
  } else {
    ln = (leaf_node_ptr) root;
  }

  for (i = 0; i < ln->num_keys; ++i) {
    if (ln->entries[i].key == k) {
      return &ln->entries[i];
    }
  }

  // Moep assertion fail
  return nullptr;
}


void CSBTree::insert(CSBTree::key_type k, CSBTree::value_type v) {
  DEBUG_FUN();
  DEBUG_MSG(k);

  // Check if the key already exists
  if (root != nullptr && find(k) != NULL) {
    return;
  }

  // Check if the tree is blank
  // Now insert the leaf node, but make sure, we take
  // one from the right level
  if (root == nullptr) {
    leaf_node_ptr n = new leaf_node();
    n->num_keys += 1;
    n->entries[0] = pair_type(k, v);

    root = (internal_node_ptr) n;

    // Statistics
    ++_num_leafs;
    ++_num_values;

    _begin = n;
    return;
  }

  // Find the first parent node of the leaf
  // we want to insert to find it with path
  _node_id id = find_leaf(k, true);
  leaf_node_ptr leaf = id.node == nullptr ? (leaf_node_ptr) root : (leaf_node_ptr) CONVERT_LEAF(id);


  // No Split
  if (leaf->num_keys < number_leaf_values) {
    DEBUG_MSG("Basic Insert: num_keys < number_leaf_values");
    // Insert into leaf
    insert_into_leaf(leaf, k, v);
  } else {

    // Enter the splitting process
    insert_into_leaf_after_splitting(id.path, leaf, k, v);
  }

  ++_num_values;
  delete id.path;
}


/*
  Modifies the input leaf and the result
*/
CSBTree::leaf_node_ptr CSBTree::split_leaf_node(CSBTree::leaf_node_ptr leaf,
    CSBTree::key_type k,
    CSBTree::value_type v) {
  DEBUG_FUN();

  // Alloc temp memory
  pair_type tmp[number_leaf_values + 1];
  unsigned cut_point = CUT(leaf->num_keys + 1);

  unsigned insertion_point, i, j;

  // Find the insertion point
  insertion_point = 0;

  while (leaf->entries[insertion_point].key < k && insertion_point < number_leaf_values) {
    ++insertion_point;
  }

  // Copy the old values + the inserted value
  for (i = 0, j = 0; i < leaf->num_keys; ++i, ++j) {
    if (j == insertion_point) {
      ++j;
    }

    tmp[j] = leaf->entries[i];
  }

  tmp[insertion_point] = pair_type(k, v);

  // Create a new leaf node and copy the values
  leaf_node_ptr result = new leaf_node();
  result->num_keys = 0;

  // Statistics
  ++_num_leafs;

  // The old leafs need to be modified
  for (i = 0; i < cut_point; ++i) {
    leaf->entries[i] = tmp[i];
  }

  // The new leaf must be modified
  for (i = cut_point; i < leaf->num_keys + 1; ++i) {
    result->entries[i - cut_point] = tmp[i];
    result->num_keys += 1;
  }

  // Modify the number of keys for the input leaf
  leaf->num_keys = cut_point;
  return result;
}



/**
 * When inserting a node and splitting it at the same time, we have to
 * check if the nodegroup size is sufficient to hold another node on the same
 * level, if this is the case SOP follows, if not, we have to reallocate the
 * nodegroup and copy the complete region plus, updating the first_child pointer
 * on the parent node (or realloc). When inserting the parent from their the
 * parent has to decide for its own.
 */
void CSBTree::insert_into_leaf_after_splitting(const CSBTree::_node_path_ptr path,
    CSBTree::leaf_node_ptr leaf,
    CSBTree::key_type k,
    CSBTree::value_type v) {
  DEBUG_FUN();
  DEBUG_MSG("Have to split for key");
  DEBUG_MSG(k);

  // Leaf is the leaf we need to split, while path keeps track of the parents
  leaf_node_ptr new_leaf = split_leaf_node(leaf, k, v);

  assert(leaf->num_keys < 100);
  assert(new_leaf->num_keys < 100);


  if (path->size() == 0) {
    DEBUG_MSG("Create new root");

    internal_node_ptr tmp = new internal_node();

    root = tmp;
    ++_num_internals;

    // Add both childs
    root->addChild(0, leaf);
    _begin = (leaf_node_ptr) root->lookup(0);

    root->addKey(new_leaf->entries[0].key);
    root->addChild(1, new_leaf);

    delete leaf;
  } else {

    internal_node_ptr parent = path->front();

    DEBUG_PRINT(leaf);
    DEBUG_PRINT(new_leaf);


    if (parent->num_keys < number_possible_keys) {
      DEBUG_MSG("Splitted leaf, parent ok");

      // The new key must be added into the parent at
      // the right position
      parent->insert_for_key(new_leaf->entries[0].key, new_leaf);

    } else {
      DEBUG_MSG("Splitted leaf, parent full");

      // If the parent has not enough space for our leaf,
      // the parent has to be split and pushed

      insert_into_parent_after_splitting(path, new_leaf, new_leaf->entries[0].key);

    }
  }

  // Do not delete leaf because it is a pointer to the real leaf
  delete new_leaf;
}

void CSBTree::insert_into_parent_after_splitting(const CSBTree::_node_path_ptr path,
    CSBTree::leaf_node_ptr first_child,
    CSBTree::key_type k) {
  DEBUG_FUN();
  DEBUG_MSG("With Leaf");
  DEBUG_MSG(k);


  unsigned i;

  // Get the path and reduce by one
  internal_node_ptr node_to_split = path->front();
  path->pop_front();

  // Find the point to split
  unsigned cut_point = CUT(node_to_split->num_keys + 1);

  // Create the new node
  internal_node_ptr new_node = new internal_node();
  ++_num_internals;

  // As a next step the keys have to be copied from left ot right
  for (i = cut_point; i < node_to_split->num_keys; ++i) {
    new_node->addChild(i - cut_point, (leaf_node_ptr) node_to_split->lookup(i));
    new_node->addKey(node_to_split->keys[i]);

    // Unset the old node
    memset(node_to_split->lookup(i), 0, cache_line_size);
  }

  // Add the last child from the previous nodes
  new_node->addChild(new_node->num_keys, (leaf_node_ptr) node_to_split->lookup(node_to_split->num_keys));

  // Set the key amount for node to split
  node_to_split->num_keys = cut_point - 1;


  // Now we have to check where the new first_child has to be inserted
  key_type pivot;

  if (k < node_to_split->keys[node_to_split->num_keys]) {
    DEBUG_MSG("LEFT");
    // Left
    node_to_split->insert_for_key(first_child->entries[0].key, first_child);
    pivot = ((leaf_node_ptr)(new_node->first_child))->entries[0].key;
  } else {
    DEBUG_MSG("RIGHT");
    // Right
    new_node->insert_for_key(first_child->entries[0].key, first_child);
    pivot = ((leaf_node_ptr)(new_node->first_child))->entries[0].key;
  }

  // Now push up
  if (path->size() == 0) {
    DEBUG_MSG("Create new root");
    DEBUG_MSG(pivot);

    // New Root
    internal_node_ptr new_root = new internal_node();
    ++_num_internals;

    new_root->setChild(0, node_to_split);
    new_root->keys[0] = pivot;
    new_root->num_keys = 1;
    new_root->setChild(1, new_node);

    leaf_node_ptr l = (leaf_node_ptr)(new_root->lookup(0)->first_child + new_root->lookup(0)->num_keys);
    leaf_node_ptr r = (leaf_node_ptr)(new_root->lookup(1)->first_child);

    l->next = r;
    r->prev = l;

    _begin = (leaf_node_ptr) new_root->lookup(0)->first_child;

    // Set the new root
    root = new_root;

    delete new_node;
    delete node_to_split;
  } else {
    DEBUG_MSG("Insert into parent");
    internal_node_ptr parent = path->front();

    if (parent->num_keys < number_possible_keys) {
      DEBUG_MSG("parent is fine");
      parent->addKey(pivot);
      parent->push_back(new_node);

    } else {
      DEBUG_MSG("Parent is full, split internal requested");
      DEBUG_MSG(pivot);
      insert_into_parent_after_splitting(path, new_node, pivot);
    }

    delete new_node;
  }


}

void CSBTree::insert_into_parent_after_splitting(const CSBTree::_node_path_ptr path,
    CSBTree::internal_node_ptr first_child,
    CSBTree::key_type k) {
  DEBUG_FUN();
  DEBUG_MSG("With Internal");

  unsigned i;

  // Get the path and reduce by one
  internal_node_ptr node_to_split = path->front();
  path->pop_front();

  // Find the point to split
  unsigned cut_point = CUT(node_to_split->num_keys + 1);

  // Create the new node
  internal_node_ptr new_node = new internal_node();
  ++_num_internals;

  // As a next step the keys have to be copied from left ot right
  for (i = cut_point; i < node_to_split->num_keys; ++i) {
    new_node->addChild(i - cut_point, node_to_split->lookup(i));
    new_node->addKey(node_to_split->keys[i]);
  }

  // Add the last child from the previous node
  new_node->addChild(new_node->num_keys, node_to_split->lookup(node_to_split->num_keys));

  // Add the last key and the corresponding child
  new_node->addKey(k);


  // connect
  new_node->push_back(first_child);

  // Set the key amount for node to split
  node_to_split->num_keys = cut_point;

  // PROB LEAK
  //delete first_child;
  key_type pivot = new_node->keys[0];

  // Now push up
  if (path->size() == 0) {
    DEBUG_MSG("Create new root");
    // New Root
    internal_node_ptr new_root = new internal_node();
    ++_num_internals;

    new_root->push_back(node_to_split);
    new_root->addKey(pivot);

    new_root->push_back(new_node);

    // Set the new root

    root = new_root;
  } else {

    DEBUG_MSG("Insert into parent");

    internal_node_ptr parent = path->front();

    if (parent->num_keys < number_possible_keys) {
      DEBUG_MSG("parent is fine");
      parent->addKey(pivot);
      parent->push_back(new_node);

    } else {

      insert_into_parent_after_splitting(path, new_node, pivot);
    }
  }

  delete new_node;
}

/*
  Inserts the key and value into the leaf with any extras
*/
void CSBTree::insert_into_leaf(CSBTree::leaf_node_ptr leaf, CSBTree::key_type k, CSBTree::value_type v) {
  unsigned insertion_point, i;
  insertion_point = 0;

  while (insertion_point < leaf->num_keys && leaf->entries[insertion_point].key < k) {
    ++insertion_point;
  }

  for (i = leaf->num_keys; i > insertion_point; --i) {
    leaf->entries[i].key = leaf->entries[i - 1].key;
    leaf->entries[i].value = leaf->entries[i - 1].value;
  }

  leaf->num_keys += 1;
  leaf->entries[insertion_point].key = k;
  leaf->entries[insertion_point].value = v;
}



void CSBTree::print(std::ostream  &w) {
  w << "----" << std::endl;

  _node_id id = find_leaf(0);
  leaf_node_ptr l = (leaf_node_ptr) CONVERT_LEAF(id);

  while (l != nullptr) {
    w << l << " ";
    w << "| ";// << std::hex << l << std::dec;

    for (unsigned i = 0; i < l->num_keys; ++i) {
      w << " " << l->entries[i].value << " ";
    }

    w << "|";
    l = l->next;
  }

  w << std::endl << "----" << std::endl;
}

size_t CSBTree::num_values() {
  return _num_values;
}

