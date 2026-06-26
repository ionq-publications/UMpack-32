// Partitioning container semantic test.
// It checks the buffer, double-buffer, and solution metadata classes used to
// store partitioning candidates and their aggregate statistics.

#include <iostream>
#include <string>

#include "partitionData.h"

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

void testPartitioningBuffer() {
  PartitioningBuffer buffer(4, 2);
  check(buffer.size() == 2 && buffer.getNumModulesUsed() == 4,
        "partitioning buffer dimensions");
  for (unsigned solution = 0; solution < buffer.size(); ++solution)
    for (unsigned module = 0; module < buffer[solution].size(); ++module)
      check(buffer[solution][module].numberPartitions() == 32,
            "partitioning buffer initializes all partition bits");

  buffer.setNumModulesUsed(3);
  buffer.setBeginUsedSoln(1);
  buffer.setEndUsedSoln(4);
  check(buffer.size() == 4 && buffer.getNumModulesUsed() == 3,
        "partitioning buffer grows solution storage");
  check(buffer.beginUsedSoln() == 1 && buffer.endUsedSoln() == 4,
        "partitioning buffer used range");
  check(buffer[3].size() == 4 && buffer[3][0].numberPartitions() == 32,
        "new solutions copy buffer shape and values");
}

void testDoubleBuffer() {
  PartitioningDoubleBuffer buffers(4, 2);
  PartitioningBuffer* mainBuffer = 0;
  PartitioningBuffer* shadowBuffer = 0;
  buffers.checkOutBuf(mainBuffer, shadowBuffer);
  check(buffers.isCheckedOut() && mainBuffer != shadowBuffer,
        "double buffer checkout");
  mainBuffer->operator[](0)[0].setToPart(3);
  PartitioningBuffer* originalMain = mainBuffer;
  PartitioningBuffer* originalShadow = shadowBuffer;
  buffers.checkInBuf(mainBuffer, shadowBuffer);
  check(!buffers.isCheckedOut(), "double buffer checkin");

  buffers.swapBuf();
  buffers.checkOutBuf(mainBuffer, shadowBuffer);
  check(mainBuffer == originalShadow && shadowBuffer == originalMain,
        "double buffer swap changes roles");
  check(shadowBuffer->operator[](0)[0].lowestNumPart() == 3,
        "double buffer swap preserves contents");
  buffers.checkInBuf(mainBuffer, shadowBuffer);
}

void testPartitioningSolutionMetadata() {
  Partitioning partitioning(3);
  partitioning[0].setToPart(0);
  partitioning[1].setToPart(1);
  partitioning[2].setToPart(1);

  std::vector<unsigned> counts(2);
  counts[0] = 1;
  counts[1] = 2;
  std::vector<std::vector<double> > areas(2, std::vector<double>(1));
  areas[0][0] = 2.0;
  areas[1][0] = 5.0;
  std::vector<unsigned> pins(2);
  pins[0] = 3;
  pins[1] = 8;

  PartitioningSolution solution(partitioning, 2, 7.0, counts, areas, pins, 9);
  check(solution.numPartitions == 2 && solution.totalArea == 7.0 &&
            solution.cost == 9,
        "partitioning solution scalar metadata");
  check(solution.partCount[1] == 2 && solution.partArea[1][0] == 5.0,
        "partitioning solution per-part metadata");
  check(solution.pinImbalance == 5, "partitioning solution pin imbalance");
}
}  // namespace

int main() {
  testPartitioningBuffer();
  testDoubleBuffer();
  testPartitioningSolutionMetadata();

  if (failures) {
    cerr << failures << " Partitioning semantic checks failed" << endl;
    return 1;
  }
  cout << "Partitioning semantic checks passed" << endl;
  return 0;
}
