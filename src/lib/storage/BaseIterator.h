#ifndef SRC_LIB_STORAGE_BASEITERATOR_H_
#define SRC_LIB_STORAGE_BASEITERATOR_H_

template<typename T>
class BaseIterator {

 public:

  virtual ~BaseIterator() {}

  virtual BaseIterator<T> *clone() = 0;

  virtual void increment() = 0;

  virtual T &dereference() const = 0;

  virtual bool equal(BaseIterator<T> *other) const = 0;

  virtual value_id_t getValueId() const = 0;

};

#endif  // SRC_LIB_STORAGE_BASEITERATOR_H_
