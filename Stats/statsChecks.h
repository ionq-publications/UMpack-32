#ifndef _STATSCHECKS_H_
#define _STATSCHECKS_H_

#include <iostream>
#include <stdexcept>

inline void statsRequire(bool condition, const char* message) {
  if (!condition) throw std::invalid_argument(message);
}

inline void statsRangeRequire(bool condition, const char* message) {
  if (!condition) throw std::out_of_range(message);
}

inline void statsRuntimeRequire(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

inline void statsWarn(bool condition, const char* message) {
  if (!condition) std::cerr << message << std::endl;
}

#endif
