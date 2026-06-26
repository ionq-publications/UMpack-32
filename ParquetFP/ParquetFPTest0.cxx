// ParquetFP semantic smoke test.
// It checks the core floorplan data structures, sequence-pair evaluation,
// BTree packing, and database round-trip I/O on small fixtures.
// This is the best executable for validating representation-level changes.

#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "DB.h"
#include "FPcommon.h"
#include "Net.h"
#include "SPeval.h"
#include "basepacking.h"
#include "btree.h"

using namespace parquetfp;
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// Semantic regression test for core floorplanning primitives.
// Checks orientation transforms, sequence-pair evaluation, and BTree/DB
// consistency against small known fixtures.
namespace {
int failures = 0;

bool near(float lhs, float rhs, float tolerance = 1e-4f) {
  return std::fabs(lhs - rhs) <=
         tolerance * std::max(1.0f, std::max(std::fabs(lhs), std::fabs(rhs)));
}

void check(bool condition, const string& message) {
  if (!condition) {
    cerr << "FAIL: " << message << endl;
    ++failures;
  }
}

void checkVector(const vector<float>& lhs, const vector<float>& rhs,
                 const string& message) {
  check(lhs.size() == rhs.size(), message + " size");
  if (lhs.size() != rhs.size()) return;
  for (unsigned i = 0; i < lhs.size(); ++i) {
    std::ostringstream detail;
    detail << message << " at index " << i << ": " << lhs[i]
           << " != " << rhs[i];
    check(near(lhs[i], rhs[i]), detail.str());
  }
}

void checkNoOverlap(const vector<float>& x, const vector<float>& y,
                    const vector<float>& width, const vector<float>& height,
                    const string& message) {
  for (unsigned i = 0; i < x.size(); ++i)
    for (unsigned j = i + 1; j < x.size(); ++j) {
      bool separate =
          x[i] + width[i] <= x[j] + 1e-5f || x[j] + width[j] <= x[i] + 1e-5f ||
          y[i] + height[i] <= y[j] + 1e-5f || y[j] + height[j] <= y[i] + 1e-5f;
      check(separate, message);
    }
}

void testOrientations() {
  const ORIENT orientations[] = {N, E, S, W, FN, FE, FS, FW};
  const float expected[][2] = {{0.25f, -0.5f}, {-0.5f, -0.25f}, {-0.25f, 0.5f},
                               {0.5f, 0.25f},  {-0.25f, -0.5f}, {-0.5f, 0.25f},
                               {0.25f, 0.5f},  {0.5f, -0.25f}};
  for (unsigned i = 0; i < 8; ++i) {
    check(toOrient(toChar(orientations[i])) == orientations[i],
          "orientation text round trip");
    pin testPin("block", false, 0.25f, -0.5f, 0);
    testPin.changeOrient(orientations[i]);
    check(near(testPin.getXOffset(), expected[i][0]) &&
              near(testPin.getYOffset(), expected[i][1]),
          "pin offset orientation transform");
  }
}

void testSequencePair() {
  vector<float> widths;
  vector<float> heights;
  widths.push_back(2);
  widths.push_back(3);
  widths.push_back(1);
  heights.push_back(1);
  heights.push_back(2);
  heights.push_back(4);

  vector<unsigned> x;
  x.push_back(0);
  x.push_back(1);
  x.push_back(2);
  do {
    vector<unsigned> y;
    y.push_back(0);
    y.push_back(1);
    y.push_back(2);
    do {
      for (unsigned direction = 0; direction < 4; ++direction) {
        bool left = (direction & 1) == 0;
        bool bottom = (direction & 2) == 0;
        SPeval normal(heights, widths, false);
        SPeval fast(heights, widths, true);
        normal.evaluate(x, y, left, bottom);
        fast.evaluate(x, y, left, bottom);
        check(near(normal.xSize, fast.xSize) && near(normal.ySize, fast.ySize),
              "normal and fast sequence-pair spans");
        checkVector(normal.xloc, fast.xloc,
                    "normal and fast sequence-pair x locations");
        checkVector(normal.yloc, fast.yloc,
                    "normal and fast sequence-pair y locations");
        checkNoOverlap(normal.xloc, normal.yloc, widths, heights,
                       "sequence-pair result must not overlap");
      }

      SPeval normal(heights, widths, false);
      SPeval fast(heights, widths, true);
      normal.evalSlacks(x, y);
      fast.evalSlacksFast(x, y);
      checkVector(normal.xSlacks, fast.xSlacks, "normal and fast x slacks");
      checkVector(normal.ySlacks, fast.ySlacks, "normal and fast y slacks");
    } while (std::next_permutation(y.begin(), y.end()));
  } while (std::next_permutation(x.begin(), x.end()));

  vector<unsigned> ordered;
  ordered.push_back(0);
  ordered.push_back(1);
  ordered.push_back(2);
  SPeval known(heights, widths, false);
  known.evaluate(ordered, ordered, true, true);
  check(near(known.xSize, 6) && near(known.ySize, 4),
        "known sequence-pair dimensions");
}

void visitTree(const vector<BTree::BTreeNode>& tree, int index, int blocks,
               vector<bool>& seen) {
  if (index == BTree::Undefined || index >= blocks) return;
  check(!seen[index], "BTree node appears once");
  if (seen[index]) return;
  seen[index] = true;
  if (tree[index].left != BTree::Undefined)
    check(tree[tree[index].left].parent == index,
          "BTree left-child parent link");
  if (tree[index].right != BTree::Undefined)
    check(tree[tree[index].right].parent == index,
          "BTree right-child parent link");
  visitTree(tree, tree[index].left, blocks, seen);
  visitTree(tree, tree[index].right, blocks, seen);
}

void checkTree(const BTree& tree, const string& message) {
  vector<bool> seen(tree.NUM_BLOCKS, false);
  int root = tree.tree[tree.NUM_BLOCKS].left;
  visitTree(tree.tree, root, tree.NUM_BLOCKS, seen);
  for (int i = 0; i < tree.NUM_BLOCKS; ++i)
    check(seen[i], message + " reaches every node");

  vector<float> x(tree.xloc().begin(), tree.xloc().begin() + tree.NUM_BLOCKS);
  vector<float> y(tree.yloc().begin(), tree.yloc().begin() + tree.NUM_BLOCKS);
  vector<float> width;
  vector<float> height;
  for (int i = 0; i < tree.NUM_BLOCKS; ++i) {
    width.push_back(tree.width(i));
    height.push_back(tree.height(i));
  }
  checkNoOverlap(x, y, width, height, message + " packing must not overlap");
}

void testBTree(const string& fixtureDir) {
  std::ifstream input((fixtureDir + "/packing.txt").c_str());
  check(input.good(), "open BTree fixture");
  HardBlockInfoType blockInfo(input, "txt");
  BTree initial(blockInfo);
  vector<BTree::BTreeNode> nodes(5);
  BTree::clean_tree(nodes);
  for (int i = 0; i < 3; ++i) {
    nodes[i].block_index = i;
    nodes[i].orient = 0;
  }
  nodes[3].left = 0;
  nodes[0].parent = 3;
  nodes[0].left = 1;
  nodes[0].right = 2;
  nodes[1].parent = 0;
  nodes[2].parent = 0;
  initial.evaluate(nodes);
  checkTree(initial, "initial BTree");

  BTree parentSwap(initial);
  parentSwap.swap(0, 1);
  checkTree(parentSwap, "parent-child BTree swap");

  BTree siblingSwap(initial);
  siblingSwap.swap(1, 2);
  checkTree(siblingSwap, "sibling BTree swap");

  BTree moved(initial);
  moved.move(1, 2, true);
  checkTree(moved, "BTree move");

  BTree rotated(initial);
  rotated.rotate(2, 1);
  check(near(rotated.width(2), 4) && near(rotated.height(2), 1),
        "BTree rotation dimensions");
  checkTree(rotated, "rotated BTree");
}

void removeRoundTrip(const string& base) {
  const char* suffixes[] = {".aux",  ".blocks", ".nodes", ".pl",
                            ".nets", ".scl",    ".wts"};
  for (unsigned i = 0; i < 7; ++i) std::remove((base + suffixes[i]).c_str());
}

bool fileExists(const string& fileName) {
  std::ifstream input(fileName.c_str());
  return input.good();
}

void testDatabase(const string& fixtureDir) {
  std::string fixtureBase = fixtureDir + "/mixed";
  DB db(fixtureBase);
  check(db.getNumNodes() == 3, "fixture node count");
  check(db.getNodes()->getNumTerminals() == 1, "fixture terminal count");
  check(db.getNets()->getNumNets() == 3 && db.getNets()->getNumPins() == 7,
        "fixture net and pin counts");
  check(near(db.getNodeWidth(0), 4) && near(db.getNodeHeight(0), 2),
        "hard block dimensions");
  check(near(db.getNodeWidth(1), 6) && near(db.getNodeHeight(1), 2),
        "oriented hard block dimensions");
  check(db.isNodeSoft(2) && near(db.getNodeWidth(2), 3) &&
            near(db.getNodeHeight(2), 3),
        "soft block dimensions and classification");

  const pin& orientedPin = db.getNets()->getNet(0).getPin(1);
  check(
      near(orientedPin.getXOffset(), 0) && near(orientedPin.getYOffset(), 0.5f),
      "parsed pin follows block orientation");
  float unweighted = db.evalHPWL(false, false);
  float weighted = db.evalHPWL(true, false);
  check(near(weighted, 2 * unweighted), "weighted HPWL uses .wts values");

  std::ostringstream name;
  name << "/tmp/parquetfp-roundtrip-" << getpid();
  string roundTripBase = name.str();
  removeRoundTrip(roundTripBase);
  db.save(roundTripBase.c_str());
  std::string reloadBase(roundTripBase);
  DB reloaded(reloadBase);
  check(reloaded.getNumNodes() == db.getNumNodes(), "round-trip node count");
  check(reloaded.getNets()->getNumNets() == db.getNets()->getNumNets(),
        "round-trip net count");
  check(near(reloaded.evalHPWL(true, false), weighted),
        "round-trip weighted HPWL");
  for (unsigned i = 0; i < db.getNumNodes(); ++i) {
    check(near(reloaded.getNodeWidth(i), db.getNodeWidth(i)) &&
              near(reloaded.getNodeHeight(i), db.getNodeHeight(i)) &&
              reloaded.getOrient(i) == db.getOrient(i),
          "round-trip dimensions and orientation");
  }
  removeRoundTrip(roundTripBase);

  string capoBase = roundTripBase + "-capo";
  removeRoundTrip(capoBase);
  BBox outline;
  db.saveCapo(capoBase.c_str(), outline, 1.0f, 30.0f);
  const char* capoSuffixes[] = {".aux", ".blocks", ".nodes",
                                ".pl",  ".nets",   ".scl"};
  for (unsigned i = 0; i < 6; ++i)
    check(fileExists(capoBase + capoSuffixes[i]),
          "Capo output artifact exists");
  removeRoundTrip(capoBase);
}
}  // namespace

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    cerr << "usage: " << argv[0] << " fixture-directory" << endl;
    return 2;
  }

  testOrientations();
  testSequencePair();
  testBTree(argv[1]);
  testDatabase(argv[1]);
  if (failures != 0) {
    cerr << failures << " ParquetFP semantic regression checks failed" << endl;
    return 1;
  }
  cout << "ParquetFP semantic regression checks passed" << endl;
  return 0;
}
