// Prim MST semantic test.
// It checks the MST cost and edge consistency on small fixed point sets,
// including singleton handling.
// Use it as the regression check for the core tree builder.

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "geomTreeBase.h"
#include "primMST.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

namespace {
int failures = 0;

void check(bool condition, const string& message) {
  if (!condition) {
    cerr << "FAIL: " << message << endl;
    ++failures;
  }
}

bool near(double lhs, double rhs) { return std::fabs(lhs - rhs) < 1e-9; }

void testGeomTreeEdge() {
  GeomTreeEdge edge(Point(3.0, -1.0), Point(-2.0, 4.0));
  check(edge.getBBox() == BBox(-2.0, -1.0, 3.0, 4.0), "tree edge bounding box");
  check(near(edge.getMDist(), 10.0), "tree edge Manhattan length");
}

void testPrimMST() {
  vector<Point> points;
  points.push_back(Point(0.0, 0.0));
  points.push_back(Point(2.0, 0.0));
  points.push_back(Point(0.0, 2.0));
  points.push_back(Point(3.0, 3.0));

  PrimMST mst(points);
  check(near(mst.getCost(), 8.0), "Prim MST cost");
  check(mst.getTree().size() == points.size(), "Prim MST node count");
  check(mst.getTree()[0].parent == 0, "Prim MST root");

  double edgeTotal = 0.0;
  for (unsigned i = 0; i < mst.getTree().size(); ++i) {
    const GeomTreeNode& node = mst.getTree()[i];
    check(node.id == static_cast<int>(i), "Prim MST node id");
    check(node.parent >= 0 && node.parent < static_cast<int>(points.size()),
          "Prim MST parent range");
    check(node.edge.first == points[i], "Prim MST edge starts at node");
    check(node.edge.second == points[node.parent],
          "Prim MST edge ends at parent");
    edgeTotal += node.edge.getMDist();
  }
  check(near(edgeTotal, mst.getCost()), "Prim MST edge sum equals cost");

  vector<Point> singleton;
  singleton.push_back(Point(7.0, 9.0));
  PrimMST onePoint(singleton);
  check(near(onePoint.getCost(), 0.0) && onePoint.getTree()[0].parent == 0,
        "single-point MST");
}
}  // namespace

int main() {
  testGeomTreeEdge();
  testPrimMST();

  if (failures) {
    cerr << failures << " GeomTrees semantic checks failed" << endl;
    return 1;
  }
  cout << "GeomTrees semantic checks passed" << endl;
  return 0;
}
