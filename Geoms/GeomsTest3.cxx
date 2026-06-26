// Interval canonicalization and merge stress test.
// It starts from an unsorted interval sequence, canonicalizes it, and then
// repeatedly merges in randomly generated sequences.
// Use it to check interval-set normalization paths.

#include <cmath>
#include <iostream>
#include <string>

#include "bbox.h"
#include "interval.h"
#include "plGeom.h"

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

void testPoints() {
  Point point(1.0, 2.0);
  point.moveBy(Point(3.0, -1.0));
  check(point == Point(4.0, 1.0), "point moveBy");
  point.scaleBy(0.5, 4.0);
  check(point == Point(2.0, 4.0), "point scaleBy");
  check(mDist(Point(1.0, 1.0), Point(4.0, 3.0)) == 5.0,
        "point Manhattan distance");
}

void testBBox() {
  BBox box(0.0, 0.0, 4.0, 2.0);
  check(near(box.getArea(), 8.0) && near(box.getHalfPerim(), 6.0),
        "bbox area and half perimeter");
  check(box.contains(Point(0.0, 0.0)) && !box.hasInside(Point(0.0, 0.0)),
        "bbox boundary containment");
  check(box.contains(BBox(1.0, 0.5, 2.0, 1.5)) &&
            !box.contains(BBox(-1.0, 0.0, 2.0, 1.0)),
        "bbox contains bbox");

  BBox other(3.0, 1.0, 6.0, 4.0);
  check(box.intersects(other), "bbox strict intersection");
  BBox overlap = box.intersect(other);
  check(overlap == BBox(3.0, 1.0, 4.0, 2.0), "bbox intersection box");
  check(near(box.mdistTo(Point(6.0, 1.0)), 2.0) &&
            near(box.mdistTo(BBox(5.0, 0.0, 7.0, 1.0)), 1.0),
        "bbox Manhattan distance");

  Point outside(6.0, -1.0);
  box.coerce(outside);
  check(outside == Point(4.0, 0.0), "bbox coerce");

  BBox centered(0.0, 0.0, 2.0, 2.0);
  centered.centerAt(Point(5.0, 5.0));
  check(centered == BBox(4.0, 4.0, 6.0, 6.0), "bbox centerAt");
  centered.ShrinkTo(0.5);
  check(centered == BBox(4.5, 4.5, 5.5, 5.5), "bbox ShrinkTo");

  RandomPoint generator(BBox(-1.0, -2.0, 3.0, 4.0), 17, 29);
  for (unsigned i = 0; i < 50; ++i) {
    Point random = generator;
    check(random.x >= -1.0 && random.x < 3.0 && random.y >= -2.0 &&
              random.y < 4.0,
          "random point bounds");
  }
}

void testIntervals() {
  Interval first(1.0, 4.0);
  Interval second(3.0, 6.0);
  check(first.doesOverlap(second) && near(first.overlap(second), 1.0),
        "interval overlap");
  check((first + 2.0) == Interval(3.0, 6.0) &&
            (second - 1.0) == Interval(2.0, 5.0),
        "interval translation");

  IntervalSeq sequence;
  sequence.push_back(Interval(5.0, 7.0));
  sequence.push_back(Interval(1.0, 3.0));
  sequence.push_back(Interval(2.5, 6.0));
  sequence.canonicalize();
  check(sequence.size() == 1 && sequence[0] == Interval(1.0, 7.0),
        "interval canonicalize merge");

  sequence.blankInterval(Interval(2.0, 5.0));
  check(sequence.size() == 2 && sequence[0] == Interval(1.0, 2.0) &&
            sequence[1] == Interval(5.0, 7.0),
        "interval blankInterval split");

  IntervalSeq other;
  other.push_back(Interval(0.0, 1.5));
  other.push_back(Interval(6.5, 8.0));
  sequence.merge(other);
  check(sequence.size() == 2 && sequence[0] == Interval(0.0, 2.0) &&
            sequence[1] == Interval(5.0, 8.0),
        "interval merge");

  IntervalSeq intersection;
  intersection.push_back(Interval(1.0, 6.0));
  sequence.intersect(intersection);
  check(sequence.size() == 2 && sequence[0] == Interval(1.0, 2.0) &&
            sequence[1] == Interval(5.0, 6.0),
        "interval intersection");
  check(near(sequence.getLength(), 2.0), "interval sequence length");
}
}  // namespace

int main() {
  testPoints();
  testBBox();
  testIntervals();

  if (failures) {
    cerr << failures << " Geoms semantic checks failed" << endl;
    return 1;
  }
  cout << "Geoms semantic checks passed" << endl;
  return 0;
}
