#ifndef SRC_LIB_HELPER_NONCOPYABLE_H_
#define SRC_LIB_HELPER_NONCOPYABLE_H_

/// Non-copyable baseclass, use to protect your class from copying
class noncopyable {
 protected:
  noncopyable() {};
  noncopyable( const noncopyable& other ) = delete; // non construction-copyable
  noncopyable& operator=( const noncopyable& ) = delete; // non copyable
};

#endif
