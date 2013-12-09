#pragma once

#include <cstdint>
#include <array>

// forward declare unique_id, so we can write a to_string
// specialization to later on befriend it
struct unique_id;

namespace std {
string to_string(const unique_id& id);
};

struct unique_id {
 public:
  unique_id() = default;
  static unique_id create();
  bool is_nil() { return value.empty(); }
 private:
  friend std::string std::to_string(const unique_id&);
  typedef std::array<std::uint8_t, 16> type;
  unique_id(type);
  type value;
};

