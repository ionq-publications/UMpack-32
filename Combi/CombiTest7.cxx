// Comprehensive Combi semantic test.
// It exercises permutation inverses, vector reordering, PartitionIds,
// randomization helpers, and Gray-code permutation coverage.
// Use this as the broad Combi behavior check.

#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "grayPart.h"
#include "grayPermut.h"
#include "partitionIds.h"
#include "permut.h"

using std::cerr;
using std::cout;
using std::endl;
using std::set;
using std::string;

namespace {
int failures = 0;

void check(bool condition, const string& message) {
  if (!condition) {
    cerr << "FAIL: " << message << endl;
    ++failures;
  }
}

string asKey(const Permutation& permutation) {
  std::ostringstream out;
  for (unsigned i = 0; i < permutation.getSize(); ++i)
    out << permutation[i] << ',';
  return out.str();
}

void testPermutationBasics() {
  const unsigned data[] = {2, 0, 1};
  Permutation permutation(3, data);
  Permutation inverse(3);
  permutation.getInverse(inverse);
  check(inverse[0] == 1 && inverse[1] == 2 && inverse[2] == 0,
        "permutation inverse");

  std::vector<unsigned> values;
  values.push_back(10);
  values.push_back(20);
  values.push_back(30);
  reorderVector(values, permutation);
  check(values[0] == 20 && values[1] == 30 && values[2] == 10, "reorderVector");
  reorderVectorBack(values, permutation);
  check(values[0] == 10 && values[1] == 20 && values[2] == 30,
        "reorderVectorBack");

  std::vector<bool> bits(3);
  bits[0] = true;
  bits[1] = false;
  bits[2] = true;
  reorderBitVector(bits, permutation);
  check(bits[0] == false && bits[1] == true && bits[2] == true,
        "reorderBitVector");
  reorderBitVectorBack(bits, permutation);
  check(bits[0] == true && bits[1] == false && bits[2] == true,
        "reorderBitVectorBack");
}

void testLexicographicStep() {
  const unsigned start[] = {0, 1, 2};
  Permutation permutation(3, start);
  check(permutation.nextLexicographic(),
        "first lexicographic successor exists");
  check(permutation[0] == 0 && permutation[1] == 2 && permutation[2] == 1,
        "first lexicographic successor value");

  unsigned steps = 1;
  while (permutation.nextLexicographic()) ++steps;
  check(steps == 5, "lexicographic enumeration count");
  check(permutation[0] == 2 && permutation[1] == 1 && permutation[2] == 0,
        "last lexicographic permutation");
  check(!permutation.nextLexicographic(),
        "last lexicographic permutation has no successor");
}

void testSortingPermutation() {
  std::vector<double> values;
  values.push_back(5.0);
  values.push_back(1.0);
  values.push_back(3.0);
  values.push_back(3.0);
  Permutation order(values, 0.0);
  check(order[0] == 3 && order[1] == 0 && order[2] == 1 && order[3] == 2,
        "sorting permutation with duplicate values");
}

void testPartitionIds() {
  PartitionIds ids;
  check(ids.isEmpty(), "empty partition ids");
  ids.setToPart(2);
  ids.addToPart(4);
  ids.addToPart(7);
  check(ids.isInPart(2) && ids.isInPart(4) && ids.isInPart(7),
        "set and add partition bits");
  check(ids.lowestNumPart() == 2, "lowest marked partition");
  check(ids.numberPartitions() == 3 && ids.numberPartitions(5) == 2,
        "partition counts");
  ids.removeFromPart(4);
  check(!ids.isInPart(4) && ids.numberPartitions() == 2,
        "remove partition bit");

  PartitionIds all;
  all.setToAll(5);
  check(all.numberPartitions() == 5 && all.isInPart(0) && all.isInPart(4) &&
            !all.isInPart(5),
        "setToAll");
  check(ids.intersectsWith(all), "partition intersection");
}

void testPartitioningRandomize() {
  Partitioning partitioning(16);
  partitioning.randomize(3);
  for (unsigned i = 0; i < partitioning.size(); ++i) {
    check(partitioning[i].numberPartitions() == 1,
          "randomize assigns one partition");
    check(partitioning[i].lowestNumPart() < 3, "randomize partition range");
  }
}

void testGrayPermutationCoverage() {
  GrayTranspositionForPermutations gray(3);
  set<string> seen;
  while (true) {
    Permutation permutation = gray.getPermutation();
    seen.insert(asKey(permutation));
    if (gray.finished()) break;
    gray.next();
  }
  check(seen.size() == 6, "gray permutation enumerates all permutations");
  check(seen.count("0,1,2,") == 1 && seen.count("2,1,0,") == 1,
        "gray permutation contains endpoints");
}

void testValidationFailures() {
  bool threw = false;
  try {
    std::vector<unsigned> badMappingData;
    badMappingData.push_back(0);
    badMappingData.push_back(1);
    Mapping mapping(3, 3, badMappingData);
    (void)mapping;
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  check(threw, "mapping rejects short initialization vector");

  threw = false;
  try {
    const unsigned badPermutationData[] = {0, 1, 3};
    Permutation permutation(3, badPermutationData);
    (void)permutation;
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  check(threw, "permutation rejects out-of-range entries");

  threw = false;
  try {
    const unsigned data[] = {2, 0, 1};
    Permutation permutation(3, data);
    std::vector<unsigned> values;
    values.push_back(10);
    values.push_back(20);
    reorderVector(values, permutation);
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  check(threw, "reorderVector rejects size mismatch");

  threw = false;
  try {
    GrayTranspositionForPermutations gray(1);
    (void)gray;
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  check(threw, "gray permutation rejects size one");

  threw = false;
  try {
    GrayCodeForPartitionings gray(3, 1);
    (void)gray;
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  check(threw, "gray partitioning rejects single partition");
}
}  // namespace

int main() {
  testPermutationBasics();
  testLexicographicStep();
  testSortingPermutation();
  testPartitionIds();
  testPartitioningRandomize();
  testGrayPermutationCoverage();
  testValidationFailures();

  if (failures) {
    cerr << failures << " Combi semantic checks failed" << endl;
    return 1;
  }
  cout << "Combi semantic checks passed" << endl;
  return 0;
}
