#ifndef _COMBICHECKS_H_
#define _COMBICHECKS_H_

#include <iostream>
#include <stdexcept>

inline void combiRequire(bool condition, const char* message) {
  if (!condition) throw std::invalid_argument(message);
}

inline void combiRangeRequire(bool condition, const char* message) {
  if (!condition) throw std::out_of_range(message);
}

inline void combiRuntimeRequire(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

inline void combiWarn(bool condition, const char* message) {
  if (!condition) std::cerr << message << std::endl;
}

#endif
