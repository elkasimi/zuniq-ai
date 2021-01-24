#include "Common.h"

struct RNG {
  RNG() : r(), engine(r()) {}

  inline int fromRange(int lo, int hi) {
    std::uniform_int_distribution<int> uniform_dist(lo, hi);
    return uniform_dist(engine);
  }

  inline int lessThan(int bound) { return fromRange(0, bound - 1); }

  random_device r;
  default_random_engine engine;
};
