// Hypergraph semantic test with terminal handling.
// It exercises the same API surface as HGraphTest2 but also verifies
// terminal counts and weight propagation through the fixed graph path.
// Use it to check the broader hypergraph statistics code.

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include "hgFixed.h"

using std::cerr;
using std::cout;
using std::endl;
using std::ofstream;
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

HGraphFixed makeGraph() {
  const int edgeOffsets[] = {0, 2, 5};
  const int connections[] = {0, 1, 1, 2, 3};
  const double edgeWeights[] = {2.5, 4.0};
  const double nodeWeights[] = {1.0, 3.0, 5.0, 7.0, 11.0};
  return HGraphFixed(edgeOffsets, connections, edgeWeights, nodeWeights, 5, 5,
                     2, false);
}

void testTopology() {
  HGraphFixed graph = makeGraph();
  check(graph.getNumNodes() == 5 && graph.getNumEdges() == 2,
        "hypergraph sizes");
  check(graph.getNumPins() == 5, "hypergraph pin count");
  check(graph.getMaxNodeDegree() == 2 && graph.getMaxEdgeDegree() == 3,
        "hypergraph maximum degrees");
  check(near(graph.getAvgNodeDegree(), 1.0) &&
            near(graph.getAvgEdgeDegree(), 2.5),
        "hypergraph average degrees");

  check(graph.getNodeByIdx(0).getDegree() == 1 &&
            graph.getNodeByIdx(1).getDegree() == 2 &&
            graph.getNodeByIdx(4).getDegree() == 0,
        "node degrees");
  check(graph.getEdgeByIdx(0).getDegree() == 2 &&
            graph.getEdgeByIdx(1).getDegree() == 3,
        "edge degrees");
  check(near(graph.getEdgeByIdx(0).getWeight(), 2.5) &&
            near(graph.getEdgeByIdx(1).getWeight(), 4.0),
        "edge weights");
  check(graph.getEdgeByIdx(1).isNodeAdjacent(&graph.getNodeByIdx(3)) &&
            !graph.getEdgeByIdx(0).isNodeAdjacent(&graph.getNodeByIdx(3)),
        "edge adjacency");
  check(graph.isConsistent(), "hypergraph incidence consistency");

  HGAlgo algorithm;
  check(algorithm.connectedComponents(graph) == 2,
        "connected components with isolated node");
}

void testWeightsAndCopy() {
  HGraphFixed graph = makeGraph();
  check(near(graph.getWeight(0), 1.0) && near(graph.getWeight(4), 11.0),
        "node weights");
  graph.setWeight(2, 13.0);
  check(near(graph.getWeight(2), 13.0), "set node weight");

  graph.setNumTerminals(2);
  check(graph.getNumTerminals() == 2 && graph.isTerminal(0) &&
            graph.isTerminal(1) && !graph.isTerminal(2),
        "terminal prefix");

  HGraphFixed copy(graph);
  check(copy.getNumNodes() == graph.getNumNodes() &&
            copy.getNumEdges() == graph.getNumEdges(),
        "hypergraph copy sizes");
  check(
      copy.getNumPins() == graph.getNumPins() && near(copy.getWeight(2), 13.0),
      "hypergraph copy data");
  check(copy.isConsistent(), "copied hypergraph consistency");
}

void testLargeWtsNetWeight() {
  const char* base = "largeWtsParserTest";
  const string auxName = string(base) + ".aux";
  const string nodesName = string(base) + ".nodes";
  const string netsName = string(base) + ".nets";
  const string wtsName = string(base) + ".wts";

  {
    ofstream aux(auxName.c_str());
    aux << "HGraph : " << nodesName << " " << netsName << " " << wtsName
        << endl;
  }
  {
    ofstream nodes(nodesName.c_str());
    nodes << "UCLA nodes 1.0" << endl
          << "NumNodes : 2" << endl
          << "NumTerminals : 0" << endl
          << "a0" << endl
          << "a1" << endl;
  }
  {
    ofstream nets(netsName.c_str());
    nets << "UCLA nets 1.0" << endl
         << "NumNets : 1" << endl
         << "NumPins : 2" << endl
         << "NetDegree : 2 net0" << endl
         << "a0 B" << endl
         << "a1 B" << endl;
  }
  {
    ofstream wts(wtsName.c_str());
    wts << "UCLA wts 1.0" << endl << "net0 12345" << endl;
  }

  const HGraphFixed graph(auxName.c_str());
  check(graph.getNumEdges() == 1, "large .wts parser graph edge count");
  check(near(graph.getEdgeByIdx(0).getWeight(), 12345.0),
        "large .wts net weight is preserved");

  std::remove(auxName.c_str());
  std::remove(nodesName.c_str());
  std::remove(netsName.c_str());
  std::remove(wtsName.c_str());
}
}  // namespace

int main() {
  testTopology();
  testWeightsAndCopy();
  testLargeWtsNetWeight();

  if (failures) {
    cerr << failures << " HGraph semantic checks failed" << endl;
    return 1;
  }
  cout << "HGraph semantic checks passed" << endl;
  return 0;
}
