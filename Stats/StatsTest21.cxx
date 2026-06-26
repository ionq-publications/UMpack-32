// Correlated tuple and summary-statistics benchmark.
// It samples a correlated normal tuple, checks the resulting summary stats,
// and compares correlation across dimensions.
// Use it to validate the higher-dimensional random-correlation helpers.

#include <cmath>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <stdexcept>
#include <string>

#include "expMins.h"
#include "multiRegre.h"
#include "stats.h"

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

bool near(double lhs, double rhs, double tolerance = 1e-9) {
  return std::fabs(lhs - rhs) < tolerance;
}

void testSummaryStats() {
  std::vector<double> data;
  data.push_back(1.0);
  data.push_back(2.0);
  data.push_back(3.0);
  data.push_back(4.0);

  TrivialStats stats(data);
  check(stats.getNum() == 4 && near(stats.getTot(), 10.0) &&
            near(stats.getAvg(), 2.5) && near(stats.getMin(), 1.0) &&
            near(stats.getMax(), 4.0),
        "trivial stats");

  TrivialStatsWithStdDev withStdDev(data);
  check(near(withStdDev.getStdDev(), std::sqrt(1.25)), "standard deviation");
}

void testCorrelationAndRegression() {
  std::vector<double> x;
  std::vector<double> y;
  for (unsigned i = 0; i < 4; ++i) {
    x.push_back(i + 1.0);
    y.push_back(2.0 * (i + 1.0) + 1.0);
  }

  Correlation corr(x, y);
  LinearRegression regression(x, y);
  check(near(double(corr), 1.0), "perfect correlation");
  check(near(regression.getK(), 2.0) && near(regression.getC(), 1.0),
        "linear regression");
}

void testFrequencyDistribution() {
  std::vector<double> data;
  data.push_back(5.0);
  data.push_back(1.0);
  data.push_back(3.0);
  data.push_back(3.0);

  CumulativeFrequencyDistribution distribution(data, 4);
  check(near(distribution.getPercentBelow(unsigned(0)), 1.0) &&
            near(distribution.getPercentBelow(unsigned(1)), 0.25) &&
            near(distribution.getPercentBelow(unsigned(2)), 0.5) &&
            near(distribution.getPercentBelow(unsigned(3)), 0.75),
        "percent below by original index");
  check(near(distribution.getValByPercentBelow(0.6), 3.0), "value by percent");
}

void testKeyCounter() {
  KeyCounter counter(5);
  counter += 2;
  counter += 2;
  counter += 4;
  counter += 7;
  check(counter[2] == 2 && counter[4] == 1 && counter[7] == 0,
        "key counter counts with threshold");

  std::ostringstream out;
  out << counter;
  check(out.str().find("    2: 2") != string::npos &&
            out.str().find("    4: 1") != string::npos,
        "key counter formatting");

  KeyCounter noThreshold(0);
  noThreshold += 7;
  check(noThreshold[7] == 1, "key counter zero threshold means unbounded");
}

void testLoadedDie() {
  std::vector<unsigned> chances;
  chances.push_back(0);
  chances.push_back(0);
  chances.push_back(7);
  RandomRawUnsigned random(123);
  LoadedDie die(chances, random);
  for (unsigned i = 0; i < 16; ++i)
    check(unsigned(die) == 2, "loaded die deterministic bucket");

  bool threw = false;
  try {
    std::vector<unsigned> invalid;
    invalid.push_back(3);
    LoadedDie invalidDie(invalid, random);
    (void)invalidDie;
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  check(threw, "loaded die rejects undersized chance vector");
}

void testExpectedMins() {
  std::vector<double> data;
  for (unsigned i = 0; i < 10; ++i) data.push_back(i + 1.0);

  ExpectedMins mins(data, 3);
  check(mins.getSize() == 3, "expected mins size");
  check(mins[1] >= mins[2], "expected mins are non-increasing");
}

void testMultipleRegression() {
  std::vector<double> x1;
  std::vector<double> x2;
  std::vector<double> y;
  for (unsigned i = 0; i < 5; ++i) {
    x1.push_back(i + 1.0);
    x2.push_back((i + 1.0) * (i + 1.0));
    y.push_back(3.0 + 2.0 * x1.back() - 0.5 * x2.back());
  }

  std::vector<std::vector<double> > predictors;
  predictors.push_back(x1);
  predictors.push_back(x2);
  std::ostringstream regressionOutput;
  std::streambuf* oldCout = std::cout.rdbuf(regressionOutput.rdbuf());
  MultipleRegression regression(predictors, y);
  std::cout.rdbuf(oldCout);
  check(near(regression.getK(0), 2.0), "multiple regression first slope");
  check(near(regression.getK(1), -0.5), "multiple regression second slope");
  check(near(regression.getC(), 3.0), "multiple regression intercept");
  check(regressionOutput.str().find("determination coeff") != string::npos,
        "multiple regression diagnostics");
}

void testValidationFailures() {
  std::vector<double> empty;
  std::ostringstream warningCapture;
  std::streambuf* oldCerr = std::cerr.rdbuf(warningCapture.rdbuf());
  TrivialStats emptyStats(empty);
  std::cerr.rdbuf(oldCerr);
  check(emptyStats.getNum() == 0, "empty trivial stats size");
  check(warningCapture.str().find("zero-sized vector") != string::npos,
        "empty trivial stats warning");

  std::vector<double> x;
  std::vector<double> y;
  x.push_back(1.0);
  y.push_back(2.0);
  y.push_back(3.0);

  bool threw = false;
  try {
    Correlation correlation(x, y);
    (void)correlation;
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  check(threw, "correlation rejects mismatched sizes");

  threw = false;
  try {
    LinearRegression regression(x, y);
    (void)regression;
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  check(threw, "linear regression rejects mismatched sizes");

  threw = false;
  try {
    RankCorrelation rankCorrelation(x, y);
    (void)rankCorrelation;
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  check(threw, "rank correlation rejects mismatched sizes");

  CumulativeFrequencyDistribution distribution(x, 1);
  threw = false;
  try {
    (void)distribution.getPercentBelow(1U);
  } catch (const std::out_of_range&) {
    threw = true;
  }
  check(threw, "frequency distribution rejects out-of-range index");

  threw = false;
  try {
    std::vector<double> small;
    for (unsigned i = 0; i < 9; ++i) small.push_back(i);
    ExpectedMins mins(small, 2);
    (void)mins;
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  check(threw, "expected mins rejects undersized samples");

  threw = false;
  try {
    ExpectedMins mins(x, 0);
    (void)mins;
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  check(threw, "expected mins rejects zero oversampling");

  threw = false;
  try {
    std::vector<std::vector<double> > predictors;
    predictors.push_back(x);
    MultipleRegression regression(predictors, y);
    (void)regression;
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  check(threw, "multiple regression rejects mismatched sizes");
}
}  // namespace

int main() {
  testSummaryStats();
  testCorrelationAndRegression();
  testFrequencyDistribution();
  testKeyCounter();
  testLoadedDie();
  testExpectedMins();
  testMultipleRegression();
  testValidationFailures();

  if (failures) {
    cerr << failures << " Stats semantic checks failed" << endl;
    return 1;
  }
  cout << "Stats semantic checks passed" << endl;
  return 0;
}
