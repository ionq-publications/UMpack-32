// Comprehensive Ctainers semantic test.
// It checks bit boards, discrete priorities, risky stacks, dense matrices,
// list containers, pair containers, and red-black trees.
// Use this as the main container-library regression check.

#include <climits>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "bitBoard.h"
#include "discretePrioritizer.h"
#include "dmatrix.h"
#include "listO1size.h"
#include "rbtree.h"
#include "riskyStack.h"
#include "umatrix.h"
#include "upair.h"

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

void testBitBoard() {
  BitBoard board(6);
  board.setBit(1);
  board.setBit(4);
  board.setBit(1);
  check(board.getNumBitsSet() == 2 && board.isBitSet(1) && board.isBitSet(4),
        "BitBoard unique set bits");
  check(board.getIndicesOfSetBits()[0] == 1 &&
            board.getIndicesOfSetBits()[1] == 4,
        "BitBoard insertion ordering");
  board.clear();
  check(
      board.getNumBitsSet() == 0 && !board.isBitSet(1) && board.getSize() == 6,
      "BitBoard sparse clear");
  board.setBit(5);
  board.reset(3);
  check(board.getSize() == 3 && board.getNumBitsSet() == 0,
        "BitBoard shrink reset");
  board.reset(9);
  check(board.getSize() == 9 && board.getNumBitsSet() == 0,
        "BitBoard grow reset");
}

void testPrioritizer() {
  vector<unsigned> priorities;
  priorities.push_back(2);
  priorities.push_back(5);
  priorities.push_back(5);
  priorities.push_back(UINT_MAX);
  priorities.push_back(2);
  DiscretePrioritizer queue(priorities);

  check(queue.top() == 2 && queue.bottom() == 4, "priority extrema");
  check(queue.headTop() == 2 && queue.next() == 1 && queue.next() == UINT_MAX,
        "priority head traversal");
  check(queue.tailTop() == 1 && queue.prev() == 2 && queue.prev() == UINT_MAX,
        "priority tail traversal");
  check(queue.head(3) == UINT_MAX && queue.tail(3) == UINT_MAX,
        "empty priority bucket");

  queue.changePriorTail(4, 7);
  check(queue.top() == 4 && queue.getMaxPriority() == 7, "priority increase");
  queue.dequeue(4);
  check(queue.top() == 2 && !queue.isEnqueued(4), "priority dequeue");
  queue.dequeueAll();
  check(queue.top() == UINT_MAX && queue.bottom() == UINT_MAX,
        "empty prioritizer");
  queue.enqueueHead(3, 9);
  check(queue.top() == 3 && queue.tailTop() == 3, "requeue after dequeueAll");
  queue.dequeue(3);
}

struct Value {
  string text;
  int number;
  Value() : text(), number(0) {}
  Value(const string& text_, int number_) : text(text_), number(number_) {}
};

void testRiskyStack() {
  RiskyStack<Value> stack(4);
  stack.push_back(Value("one", 1));
  stack.push_back(Value("two", 2));

  RiskyStack<Value> copied(stack);
  copied.push_back(Value("three", 3));
  check(copied.size() == 3 && copied[0].text == "one" &&
            copied.back().number == 3,
        "RiskyStack copy preserves spare capacity and values");

  RiskyStack<Value> assigned;
  assigned = copied;
  assigned = assigned;
  check(assigned.size() == 3 && assigned[1].text == "two",
        "RiskyStack assignment and self-assignment");
  assigned.reserve(8);
  assigned.push_back(Value("four", 4));
  check(assigned.size() == 4 && assigned.back().text == "four",
        "RiskyStack reserve");
  assigned.pop_back();
  check(assigned.back().number == 3, "RiskyStack pop");
}

void testMatrices() {
  DenseMatrix dense(2, 3, 1.5);
  dense(0, 2) = 7.0;
  dense.setDiag(4.0);
  check(dense.getNumRows() == 2 && dense.getNumCols() == 3 &&
            dense(0, 0) == 4.0 && dense(1, 1) == 4.0 && dense(0, 2) == 7.0,
        "DenseMatrix dimensions, indexing, and diagonal");
  DenseMatrix denseCopy(dense);
  dense(0, 0) = 8.0;
  check(denseCopy(0, 0) == 4.0, "DenseMatrix deep copy");

  unsigned initial[] = {1, 2, 3, 4, 5, 6};
  UDenseMatrix unsignedMatrix(3, 2, initial, 0);
  unsignedMatrix.setDiag(9);
  check(unsignedMatrix(0, 0) == 9 && unsignedMatrix(1, 1) == 9 &&
            unsignedMatrix(2, 1) == 6,
        "UDenseMatrix rectangular diagonal");
  UDenseMatrix unsignedCopy(unsignedMatrix);
  unsignedMatrix.setToZero();
  check(unsignedMatrix(2, 1) == 0 && unsignedCopy(2, 1) == 6,
        "UDenseMatrix zeroing and deep copy");
}

void testListsAndPairs() {
  listO1size<int> values;
  values.push_back(2);
  values.push_front(1);
  listO1size<int>::iterator position = values.end();
  values.insert(position, 3);
  check(values.size() == 3, "listO1size insertions");
  position = values.begin();
  ++position;
  values.erase(position);
  values.pop_front();
  check(values.size() == 1 && values.front() == 3, "listO1size removals");
  values.clear();
  check(values.size() == 0, "listO1size clear");

  slistO1size<int> shortList;
  shortList.push_front(1);
  shortList.push_front(2);
  shortList.pop_front();
  check(shortList.size() == 1 && shortList.front() == 1,
        "slistO1size operations");

  UPair pair(4, 9);
  UPairs pairs(2, pair);
  check(pairs.size() == 2 && pairs[1].first == 4 && pairs[1].second == 9,
        "UPair and UPairs construction");
}

void testRBTree() {
  RBTreeNode* plusOne = new RBTreeNode(1.0, 1);
  RBTreeNode* root = plusOne;
  RBTreeNode* minusFive = new RBTreeNode(5.0, -1);
  RBTreeNode* plusTwo = new RBTreeNode(2.0, 1);
  RBTreeNode* minusFour = new RBTreeNode(4.0, -1);
  root = root->insert(minusFive);
  root = root->insert(plusTwo);
  root = root->insert(minusFour);
  check(root->check(NULL) > 0, "RBTree invariants after insertion");
  check(root->find(1, 1.0) == plusOne && root->find(-1, 5.0) == minusFive,
        "RBTree lookup");
  check(
      plusOne->successor() == plusTwo && minusFive->predecessor() == minusFour,
      "RBTree successor and predecessor");

  RBInterval minimum = root->findMin(RBInterval(0.0, 10.0));
  check(std::fabs(minimum.low - 2.0) < 1e-9 &&
            std::fabs(minimum.high - 4.0) < 1e-9,
        "RBTree minimum interval");
  int slope0 = root->findSlope(0.0);
  int slope3 = root->findSlope(3.0);
  int slope6 = root->findSlope(6.0);
  std::ostringstream slopeMessage;
  slopeMessage << "RBTree slope evaluation: " << slope0 << ", " << slope3
               << ", " << slope6;
  check(slope0 == -2 && slope3 == 0 && slope6 == 2, slopeMessage.str());
  check(std::fabs(root->findCost(0.0) - 3.0) < 1e-9 &&
            std::fabs(root->findCost(3.0)) < 1e-9 &&
            std::fabs(root->findCost(6.0) - 3.0) < 1e-9,
        "RBTree cost evaluation");

  RBTreeNode* copied = new RBTreeNode(*root);
  check(
      copied->check(NULL) > 0 && std::fabs(copied->findCost(6.0) - 3.0) < 1e-9,
      "RBTree deep copy");
  delete copied;

  root = root->findAndDelete(1, 2.0);
  check(root->find(1, 2.0) == NULL && root->check(NULL) > 0,
        "RBTree internal deletion");
  root = root->findAndDelete(-1, 5.0);
  check(root->find(-1, 5.0) == NULL && root->check(NULL) > 0,
        "RBTree leaf deletion");
  delete root;
}
}  // namespace

int main() {
  testBitBoard();
  testPrioritizer();
  testRiskyStack();
  testMatrices();
  testListsAndPairs();
  testRBTree();
  if (failures) {
    cerr << failures << " Ctainers semantic checks failed" << endl;
    return 1;
  }
  cout << "Ctainers semantic checks passed" << endl;
  return 0;
}
