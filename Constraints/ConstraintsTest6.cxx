// Constraint semantic test.
// It checks individual constraint behavior, then exercises the collection
// interface by compactifying, querying bit-vectors, and enforcing all
// constraints on a sample placement.

#include <cmath>
#include <iostream>
#include <string>

#include "allConstr.h"
#include "constraints.h"

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

bool near(double lhs, double rhs) { return std::fabs(lhs - rhs) < 1e-9; }

Placement makePlacement() {
  Placement placement(5);
  placement[0] = Point(0.0, 0.0);
  placement[1] = Point(1.0, 1.0);
  placement[2] = Point(3.0, 0.0);
  placement[3] = Point(5.0, 7.0);
  placement[4] = Point(8.0, 2.0);
  return placement;
}

void testIndividualConstraints() {
  Placement placement = makePlacement();

  {
    const unsigned data[] = {1, 3};
    Mapping mapping(2, placement.getSize(), data);
    SubPlacement subset(placement, mapping);
    Placement fixedLocs(2);
    fixedLocs[0] = Point(2.0, 2.0);
    fixedLocs[1] = Point(4.0, 4.0);
    FixedGroupConstraint fixed(subset, fixedLocs);
    check(!fixed.isSatisfied(), "fixed group initially unsatisfied");
    fixed.enforce();
    check(fixed.isSatisfied() && placement[1] == Point(2.0, 2.0) &&
              placement[3] == Point(4.0, 4.0),
          "fixed group enforce");
  }

  {
    const unsigned data[] = {0, 2};
    Mapping mapping(2, placement.getSize(), data);
    SubPlacement subset(placement, mapping);
    RectRegionConstraint region(subset, BBox(0.5, 0.5, 2.5, 2.5));
    check(!region.isSatisfied() && near(region.getPenalty(), 1.0),
          "rect region penalty");
    region.enforce();
    check(region.isSatisfied() && placement[0] == Point(0.5, 0.5) &&
              placement[2] == Point(2.5, 0.5),
          "rect region enforce");
  }

  {
    const unsigned data[] = {1, 4};
    Mapping mapping(2, placement.getSize(), data);
    SubPlacement subset(placement, mapping);
    TetheredGroupConstraint tethered(
        subset, Placement(subset.getMapping(), placement), 0.0);
    check(tethered.isSatisfied() && near(tethered.getPenalty(), 0.0),
          "tethered group satisfied at anchor");
  }

  {
    const unsigned data[] = {0, 1, 2};
    Mapping mapping(3, placement.getSize(), data);
    SubPlacement subset(placement, mapping);
    EqualXConstraint equalX(subset);
    check(!equalX.isSatisfied(), "equal x initially unsatisfied");
    equalX.enforce();
    check(equalX.isSatisfied() && near(placement[0].x, placement[1].x) &&
              near(placement[1].x, placement[2].x),
          "equal x enforce");
  }

  {
    const unsigned data[] = {0, 1, 2};
    Mapping mapping(3, placement.getSize(), data);
    SubPlacement subset(placement, mapping);
    SoftGroupConstraint soft(subset, 10.0, 10.0, 20.0);
    check(soft.isSatisfied() && soft.isSoft(), "soft group satisfied");
    StayTogetherConstraint together(subset, BBox(-0.5, -0.5, 0.5, 0.5));
    together.enforce();
    check(together.isSatisfied() && placement[0] == placement[1] &&
              placement[1] == placement[2],
          "stay together enforce");
  }
}

void testConstraintCollection() {
  Placement placement(5);
  placement[0] = Point(0.0, 0.0);
  placement[1] = Point(10.0, 0.0);
  placement[2] = Point(2.0, 1.0);
  placement[3] = Point(0.0, 5.0);
  placement[4] = Point(1.0, 9.0);

  const unsigned fixedData[] = {1};
  const unsigned fixedXData[] = {3};
  const unsigned fixedYData[] = {4};
  const unsigned regionData[] = {0, 2};

  Mapping fixedMap(1, placement.getSize(), fixedData);
  Mapping fixedXMap(1, placement.getSize(), fixedXData);
  Mapping fixedYMap(1, placement.getSize(), fixedYData);
  Mapping regionMap(2, placement.getSize(), regionData);

  SubPlacement fixedSubset(placement, fixedMap);
  SubPlacement fixedXSubset(placement, fixedXMap);
  SubPlacement fixedYSubset(placement, fixedYMap);
  SubPlacement regionSubset(placement, regionMap);

  Placement fixedLocs(1);
  fixedLocs[0] = Point(1.0, 1.0);
  Placement fixedXLocs(1);
  fixedXLocs[0] = Point(4.0, 100.0);
  Placement fixedYLocs(1);
  fixedYLocs[0] = Point(100.0, 6.0);

  Constraints constraints;
  constraints.add(FixedGroupConstraint(fixedSubset, fixedLocs));
  constraints.add(FixedXGroupConstraint(fixedXSubset, fixedXLocs));
  constraints.add(FixedYGroupConstraint(fixedYSubset, fixedYLocs));
  constraints.add(RectRegionConstraint(regionSubset, BBox(0.5, 0.5, 2.5, 1.5)));
  constraints.compactify();

  check(constraints.hasFGC() && constraints.hasFXGC() && constraints.hasFYGC(),
        "constraint collection retains fixed constraint groups");

  Permutation permutation;
  constraints.getPermutation(permutation);
  check(permutation[1] == 0 && permutation[3] == 1 && permutation[4] == 2,
        "constraint permutation fixed prefixes");

  bit_vector fixedBits;
  bit_vector fixedXBits;
  bit_vector fixedYBits;
  constraints.getFGCbitVec(fixedBits);
  constraints.getFXGCbitVec(fixedXBits);
  constraints.getFYGCbitVec(fixedYBits);
  check(fixedBits[1] && !fixedBits[0] && fixedXBits[3] && fixedYBits[4],
        "constraint bit vectors");

  constraints.enforceAll();
  check(constraints.allSatisfied(), "constraint collection enforceAll");
  check(placement[1] == Point(1.0, 1.0),
        "fixed group location after enforceAll");
  check(near(placement[3].x, 4.0) && near(placement[3].y, 5.0),
        "fixed x location after enforceAll");
  check(near(placement[4].x, 1.0) && near(placement[4].y, 6.0),
        "fixed y location after enforceAll");
  check(placement[0] == Point(0.5, 0.5) && placement[2] == Point(2.0, 1.0),
        "region constraint after enforceAll");
}
}  // namespace

int main() {
  testIndividualConstraints();
  testConstraintCollection();

  if (failures) {
    cerr << failures << " Constraints semantic checks failed" << endl;
    return 1;
  }
  cout << "Constraints semantic checks passed" << endl;
  return 0;
}
