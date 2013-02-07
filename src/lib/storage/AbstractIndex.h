/** @file AbstractIndex.h
 *
 * Contains class definition for InvertedIndex Base Class
 *
 */

#ifndef SRC_LIB_STORAGE_ABSTRACTINDEX_H_
#define SRC_LIB_STORAGE_ABSTRACTINDEX_H_


class AbstractIndex {

public:
  /**
   * Destructor.
   */
  virtual ~AbstractIndex();

  virtual void shrink() = 0;

};

#endif  // SRC_LIB_STORAGE_ABSTRACTINDEX_H_
