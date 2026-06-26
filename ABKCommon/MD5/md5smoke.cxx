#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "../abkMD5.h"

namespace {

void require(bool cond, const char* msg) {
  if (!cond) {
    std::cerr << "md5smoke failed: " << msg << std::endl;
    std::exit(1);
  }
}

}  // namespace

int main() {
  MD5 md5("abc", true);
  std::ostringstream oss;
  oss << md5;
  require(oss.str() == "900150983cd24fb0d6963f7d28e17f72", "abc digest");

  MD5 split;
  split.update("a", 1, false);
  split.update("b", 1, false);
  split.update("c", 1, true);
  std::ostringstream oss2;
  oss2 << split;
  require(oss2.str() == oss.str(), "incremental digest");

  require(split.getWord(0) == md5.getWord(0), "word 0");
  require(split.getWord(1) == md5.getWord(1), "word 1");
  require(split.getWord(2) == md5.getWord(2), "word 2");
  require(split.getWord(3) == md5.getWord(3), "word 3");

  return 0;
}
