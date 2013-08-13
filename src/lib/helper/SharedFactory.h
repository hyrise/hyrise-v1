#ifndef SRC_LIB_HELPER_SHAREDFACTORY_H
#define SRC_LIB_HELPER_SHAREDFACTORY_H

#include <memory>

template <typename T>
class SharedFactory : public std::enable_shared_from_this<T> {
 public:
  template <typename... ARGS>
  static std::shared_ptr<T> create(ARGS... args) {
    return std::make_shared<T>(std::forward<ARGS>(args)...);
  }
};


#endif
