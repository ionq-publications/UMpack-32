// Placement semantic test.
// It checks polygon geometry, reordering, coordinate transforms, and layout
// box lookup behavior with explicit assertions.

#include <cmath>
#include <iostream>
#include <string>

#include "layoutBBoxes.h"
#include "placement.h"
#include "transf.h"

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

Placement makeSquare() {
  Placement placement(4);
  placement[0] = Point(0.0, 0.0);
  placement[1] = Point(2.0, 0.0);
  placement[2] = Point(2.0, 2.0);
  placement[3] = Point(0.0, 2.0);
  return placement;
}

void testPlacementGeometry() {
  Placement placement = makeSquare();
  check(placement.getBBox() == BBox(0.0, 0.0, 2.0, 2.0),
        "placement bounding box");
  check(placement.getCenterOfGravity() == Point(1.0, 1.0),
        "placement center of gravity");
  check(near(placement.getPolygonArea(), 4.0),
        "counter-clockwise polygon area");
  check(placement.isInsidePolygon(Point(1.0, 1.0)), "point inside polygon");
  check(!placement.isInsidePolygon(Point(3.0, 1.0)), "point outside polygon");

  Placement reversed(4);
  reversed[0] = placement[0];
  reversed[1] = placement[3];
  reversed[2] = placement[2];
  reversed[3] = placement[1];
  check(near(reversed.getPolygonArea(), -4.0), "clockwise polygon area");

  Placement shifted = placement;
  shifted[2] = Point(5.0, 3.0);
  check(near(shifted - placement, 4.0), "maximum pointwise Manhattan distance");
}

void testReorderingAndExamples() {
  Placement placement(3);
  placement[0] = Point(10.0, 0.0);
  placement[1] = Point(20.0, 1.0);
  placement[2] = Point(30.0, 2.0);
  const unsigned data[] = {2, 0, 1};
  Permutation permutation(3, data);
  placement.reorder(permutation);
  check(placement[0].x == 20.0 && placement[1].x == 30.0 &&
            placement[2].x == 10.0,
        "placement reorder");
  placement.reorderBack(permutation);
  check(placement[0].x == 10.0 && placement[1].x == 20.0 &&
            placement[2].x == 30.0,
        "placement reorderBack");

  Placement grid(Placement::_Grid2, 3, BBox(2.0, 4.0, 6.0, 8.0));
  check(grid.getSize() == 9 && grid.getBBox() == BBox(2.0, 4.0, 6.0, 8.0),
        "grid example size and bounds");
}

void testTransforms() {
  Point point(2.0, 3.0);
  SwapXY swap;
  swap.apply(point);
  check(point == Point(3.0, 2.0), "swap coordinates");

  BBoxToBBox scale(BBox(0.0, 0.0, 2.0, 4.0), BBox(10.0, 20.0, 14.0, 28.0));
  point = Point(1.0, 3.0);
  scale.apply(point);
  check(point == Point(12.0, 26.0), "bbox-to-bbox transform");

  point = Point(3.0, 4.0);
  ToPolar toPolar;
  ToXY toXY;
  toPolar.apply(point);
  check(near(point.x, 5.0), "polar radius");
  toXY.apply(point);
  check(near(point.x, 3.0) && near(point.y, 4.0), "polar round trip");
}

void testLayoutBBoxes() {
  LayoutBBoxes boxes;
  boxes.push_back(BBox(0.0, 0.0, 1.0, 1.0));
  boxes.push_back(BBox(2.0, 2.0, 4.0, 4.0));
  check(boxes.contains(Point(0.5, 0.5)) && boxes.locate(Point(0.5, 0.5)) == 0,
        "locate point in first layout box");
  check(boxes.contains(Point(3.0, 3.0)) && boxes.locate(Point(3.0, 3.0)) == 1,
        "locate point in second layout box");
  check(!boxes.contains(Point(1.5, 1.5)) &&
            boxes.locate(Point(1.5, 1.5)) == boxes.size(),
        "point outside layout boxes");
}
}  // namespace

int main() {
  testPlacementGeometry();
  testReorderingAndExamples();
  testTransforms();
  testLayoutBBoxes();

  if (failures) {
    cerr << failures << " Placement semantic checks failed" << endl;
    return 1;
  }
  cout << "Placement semantic checks passed" << endl;
  return 0;
}
