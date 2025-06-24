// Cache for sensor.

#ifndef MAIN_CACHE_
#define MAIN_CACHE_

#include <string.h>

template <class T, int L> class Cache {
public:
  Cache() { memset(data_, 0, sizeof(data_)); }
  inline void put(T value) {
    current_ = (current_ + 1) % L;
    data_[current_] = value;
  }
  inline T get(int offset = 0) const { return data_[(current_ + offset) % L]; }

  inline void Clear() {
    memset(data_, 0, sizeof(data_));
  }

private:
  T data_[L];
  int current_ = 0;
};

#endif // MAIN_CACHE_