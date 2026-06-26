// Comprehensive semantic regression test.
// It checks command-line parsing, verbosity, gcd/factorial helpers, hashing,
// RNG repeatability, timers, stream parsing, memory-mapped input, and
// message ordering.
// Use this as the main ABKCommon behavioral smoke test.

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "abkCRC32.h"
#include "abkMD5.h"
#include "abkcommon.h"
#include "abkmessagebuf.h"
#include "mmapIStream.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;

namespace {
int failures = 0;

void check(bool condition, const string& message) {
  if (!condition) {
    cerr << "FAIL: " << message << endl;
    ++failures;
  }
}

void testParams() {
  const char* argv[] = {"semantic", "+flag",   "-count", "-7",    "-unsigned",
                        "42",       "--ratio", "2.5",    "-name", "value"};
  const int argc = sizeof(argv) / sizeof(argv[0]);
  BoolParam flag("flag", argc, argv);
  IntParam count("count", argc, argv);
  UnsignedParam unsignedValue("unsigned", argc, argv);
  DoubleParam ratio("ratio", argc, argv);
  StringParam name("name", argc, argv);
  check(flag.found() && flag.on(), "positive boolean parameter");
  check(count.found() && int(count) == -7, "integer parameter");
  check(unsignedValue.found() && unsigned(unsignedValue) == 42,
        "unsigned parameter");
  check(ratio.found() && std::fabs(double(ratio) - 2.5) < 1e-12,
        "double parameter");
  check(name.found() && string(name) == "value", "string parameter");

  const char* noArgs[] = {"semantic"};
  check(NoParams(1, noArgs).found(), "empty command line detection");
  const char* offArgs[] = {"semantic", "-flag"};
  BoolParam off("flag", 2, offArgs);
  check(off.found() && !off.on(), "negative boolean parameter");
}

void testVerbosity() {
  Verbosity silent("silent");
  check(silent.getNumTypes() == 3 && silent[0] == 0 && silent[1] == 0 &&
            silent[2] == 0,
        "silent verbosity");
  Verbosity levels("2_3_4_5");
  check(levels.getNumTypes() == 4 && levels[0] == 2 && levels[3] == 5,
        "verbosity string parsing");
  levels.setForMajStats(9);
  check(levels.getForMajStats() == 9, "verbosity mutation");
}

void testFunctions() {
  check(abkGcd(84, 30) == 6 && abkGcd(0, 9) == 9, "two-argument gcd");
  std::vector<unsigned> values;
  values.push_back(24);
  values.push_back(36);
  values.push_back(60);
  check(abkGcd(values) == 12, "vector gcd");
  check(abkFactorial(0) == 1 && abkFactorial(1) == 1 && abkFactorial(5) == 120,
        "factorial boundary cases");
  check(equalDouble(1.0, 1.0 + 1e-8) && !equalDouble(1.0, 1.1),
        "double comparison helpers");
  check(equalFloat(1.0f, 1.0f + 1e-7f) && !equalFloat(1.0f, 1.1f),
        "float comparison helpers");

  char path[] = "/tmp/example.data.txt";
  check(string(BaseFileName(path)) == "example.data",
        "base filename extraction");
  check(string(SgnPartOfFileName("one/two/three.cxx")) == "two/three.cxx",
        "significant filename extraction");
}

void testHashes() {
  std::ostringstream md5;
  md5 << MD5("abc");
  check(md5.str() == "900150983cd24fb0d6963f7d28e17f72", "MD5 known vector");
  check(unsigned(CRC32("abc")) == 1686944627U, "CRC32 known vector");

  MD5 incremental;
  incremental.update("a", false);
  incremental.update("bc", true);
  std::ostringstream incrementalText;
  incrementalText << incremental;
  check(incrementalText.str() == md5.str(), "incremental MD5");
}

void testRandom() {
  RandomUnsigned first(10, 20, 1234);
  RandomUnsigned second(10, 20, 1234);
  for (unsigned i = 0; i < 100; ++i) {
    unsigned lhs = first;
    unsigned rhs = second;
    check(lhs == rhs, "equal RNG seeds produce equal values");
    check(lhs >= 10 && lhs <= 20, "unsigned RNG bounds");
  }

  RandomDouble doubles(-2.0, 3.0, 99);
  for (unsigned i = 0; i < 100; ++i) {
    double value = doubles;
    check(value >= -2.0 && value < 3.0, "double RNG bounds");
  }

  RandomRawUnsigned kernel250(7);
  RandomRawUnsigned1279 kernel1279(7);
  bool differ = false;
  for (unsigned i = 0; i < 20; ++i)
    if (unsigned(kernel250) != unsigned(kernel1279)) differ = true;
  check(differ, "RNG kernels produce distinct sequences");
}

void testTimer() {
  Timer timer;
  check(!timer.isStopped(), "timer starts running");
  timer.stop();
  check(
      timer.isStopped() && timer.getUserTime() >= 0 && timer.getRealTime() >= 0,
      "timer stop and nonnegative readings");
  timer.start(1.0);
  check(!timer.isStopped() && !timer.expired(), "timer restart and limit");
  timer.stop();
}

void testStreams(const string& tempDir) {
  std::istringstream parsed("# comment\n 12 and token\n");
  int line = 1;
  unsigned number = 0;
  string token;
  parsed >> eathash(line) >> number >> needword("and", line) >> token >>
      needeol(line);
  check(number == 12 && token == "token" && line == 2,
        "stream parsing helpers");

  string fileName = tempDir + "/mmap-input.txt";
  {
    std::ofstream output(fileName.c_str());
    output << "mapped 73\nsecond line\n";
  }
  {
    MMapIStream input(fileName.c_str());
    string word;
    unsigned value = 0;
    input >> word >> value;
    check(word == "mapped" && value == 73, "memory-mapped input stream");
  }
  std::remove(fileName.c_str());
}

void testMessageBuffer() {
  ABKMessageBuf buffer;
  buffer.addMessage(1, "low");
  buffer.addMessage(3, "high");
  buffer.addMessage(2, "middle");
  std::ostringstream output;
  output << buffer;
  check(output.str() == "highmiddlelow", "message priority ordering");
}
}  // namespace

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    cerr << "usage: " << argv[0] << " temporary-directory" << endl;
    return 2;
  }

  SeedHandler::turnOffLogging();
  testParams();
  testVerbosity();
  testFunctions();
  testHashes();
  testRandom();
  testTimer();
  testStreams(argv[1]);
  testMessageBuffer();

  if (failures) {
    cerr << failures << " ABKCommon semantic checks failed" << endl;
    return 1;
  }
  cout << "ABKCommon semantic checks passed" << endl;
  return 0;
}
