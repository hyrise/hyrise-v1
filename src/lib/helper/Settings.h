#ifndef SRC_LIB_HELPER_SETTINGS_H_
#define SRC_LIB_HELPER_SETTINGS_H_

#include <cstddef>

/*  Singleton data container class for global settings.
    Use SettingsOperation to manipulate global settings as long as units
    implementing certain decisions are missing. */
class Settings {
  size_t threadpoolSize;
  Settings() :
      threadpoolSize(1)
  {}
 public:
  ~Settings() {}
  static Settings *getInstance() {
    static Settings *instance = nullptr;
    if (instance == nullptr)
      instance = new Settings();
    return instance;
  }

  //  Control the maximum number of parallel executable operation tasks.
  size_t getThreadpoolSize() const;
  void setThreadpoolSize(const size_t newSize);
};

#endif  // SRC_LIB_HELPER_SETTINGS_H_

