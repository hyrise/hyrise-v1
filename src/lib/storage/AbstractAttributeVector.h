#ifndef SRC_LIB_STORAGE_ABSTRACTATTRIBUTEVECTOR_H_
#define SRC_LIB_STORAGE_ABSTRACTATTRIBUTEVECTOR_H_

class AbstractAttributeVector {
 public:

  virtual ~AbstractAttributeVector() {};

  virtual void *data() = 0;
  virtual void setNumRows(size_t s) = 0;

};

#endif  // SRC_LIB_STORAGE_ABSTRACTATTRIBUTEVECTOR_H_
