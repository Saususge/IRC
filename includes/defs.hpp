#ifndef DEFS_HPP
#define DEFS_HPP
#include <cstddef>

struct ClientID {
  size_t value;
  explicit ClientID(size_t v) : value(v) {}
  operator size_t() const { return value; }

  ClientID operator++(int) {
    ClientID temp = *this;
    this->value++;
    return temp;
  }
};
struct SessionID {
  size_t value;
  explicit SessionID(size_t v) : value(v) {}
  operator size_t() const { return value; }

  SessionID operator++(int) {
    SessionID temp = *this;
    this->value++;
    return temp;
  }
};
#endif