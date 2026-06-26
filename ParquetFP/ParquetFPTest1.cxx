// ParquetFP visualization unit test.
// Calls DB::plot() with each combination of plotSlacks/plotNets/plotNames and
// verifies the resulting gnuplot script has the correct number of block
// rectangles, label lines, EOF markers, and data sections.

/**************************************************************************
***
*** Copyright (c) 2000-2006 Regents of the University of Michigan,
***               Saurabh N. Adya, Hayward Chan, Jarrod A. Roy
***               and Igor L. Markov
***
***  Contact author(s): sadya@umich.edu, igor.markov1@gmail.com
***  Original Affiliation:   University of Michigan, EECS Dept.
***                          Ann Arbor, MI 48109-2122 USA
***
***  Permission is hereby granted, free of charge, to any person obtaining
***  a copy of this software and associated documentation files (the
***  "Software"), to deal in the Software without restriction, including
***  without limitation the rights to use, copy, modify, merge, publish,
***  distribute, sublicense, and/or sell copies of the Software, and to
***  permit persons to whom the Software is furnished to do so, subject to
***  the following conditions:
***
***  The above copyright notice and this permission notice shall be included
***  in all copies or substantial portions of the Software.
***
*** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
*** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
*** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
*** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
*** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*** SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***
***
***************************************************************************/

#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "DB.h"
#include "FPcommon.h"

using namespace parquetfp;
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

// Structural properties extracted from a gnuplot script written by DB::plot().
struct PlotContent {
  bool hasGnuplotHeader;
  bool hasNoKey;
  bool hasSizeRatio;
  bool hasTitleLine;
  bool hasPauseLine;
  int eofCount;
  int plotDataSections;  // number of '-' data sets in the plot command
  int blockRectangles;   // closed rectangles in the first data section
  int nameLabels;        // set label "blockname" lines
  int slackLabels;       // set label "x ..." and "y ..." slack lines

  PlotContent()
      : hasGnuplotHeader(false),
        hasNoKey(false),
        hasSizeRatio(false),
        hasTitleLine(false),
        hasPauseLine(false),
        eofCount(0),
        plotDataSections(0),
        blockRectangles(0),
        nameLabels(0),
        slackLabels(0) {}
};

PlotContent parsePlotFile(const string& fileName) {
  PlotContent c;
  std::ifstream in(fileName.c_str());
  if (!in.good()) return c;

  bool inFirstDataSection = false;
  bool pastFirstEOF = false;
  int coordsInCurrentGroup = 0;

  string line;
  while (std::getline(in, line)) {
    if (line.find("Use this file as a script for gnuplot") != string::npos)
      c.hasGnuplotHeader = true;
    else if (line == "set nokey")
      c.hasNoKey = true;
    else if (line.find("set size ratio") != string::npos)
      c.hasSizeRatio = true;
    else if (line.find("set title") != string::npos)
      c.hasTitleLine = true;
    else if (line.find("pause -1") != string::npos)
      c.hasPauseLine = true;
    else if (line == "EOF") {
      ++c.eofCount;
      inFirstDataSection = false;
      pastFirstEOF = true;
    } else if (line.find("plot '-'") != string::npos) {
      inFirstDataSection = true;
      c.plotDataSections = 1;
      for (size_t i = 0; i < line.size(); ++i)
        if (line[i] == ',') ++c.plotDataSections;
    } else if (line.find("set label") != string::npos) {
      // Slack labels contain "x <float>" or "y <float>" as the label text.
      // Name labels contain block names (none of which start with x/y in
      // the mixed fixture).
      if (line.find("\"x ") != string::npos ||
          line.find("\"y ") != string::npos)
        ++c.slackLabels;
      else
        ++c.nameLabels;
    } else if (inFirstDataSection && !pastFirstEOF) {
      if (line.empty()) {
        if (coordsInCurrentGroup > 0) {
          ++c.blockRectangles;
          coordsInCurrentGroup = 0;
        }
      } else {
        float fx, fy;
        std::istringstream ss(line);
        if (ss >> fx >> fy) ++coordsInCurrentGroup;
      }
    }
  }
  // Handle a final group with no trailing blank line.
  if (coordsInCurrentGroup > 0) ++c.blockRectangles;

  return c;
}

string makeTmpName(const string& tag) {
  std::ostringstream name;
  name << "/tmp/parquetfp-plot-test-" << getpid() << "-" << tag << ".gpl";
  return name.str();
}

// mixed fixture: 3 nodes (hard0, hard1, soft0) + 1 terminal (term0), 3 nets.

void testBasic(DB& db) {
  string fname = makeTmpName("basic");
  db.plot(fname.c_str(), 100.0f, 20.0f, 1.0f, 0.5f, 50.0f,
          /*plotSlacks=*/false, /*plotNets=*/false, /*plotNames=*/false);
  PlotContent c = parsePlotFile(fname);

  check(c.hasGnuplotHeader, "basic: gnuplot header present");
  check(c.hasNoKey, "basic: set nokey present");
  check(c.hasSizeRatio, "basic: set size ratio present");
  check(c.hasTitleLine, "basic: set title line present");
  check(c.hasPauseLine, "basic: pause line present");
  check(c.blockRectangles == 3, "basic: 3 block rectangles drawn");
  check(c.nameLabels == 0, "basic: no name labels");
  check(c.slackLabels == 0, "basic: no slack labels");
  check(c.eofCount == 1, "basic: exactly one EOF");
  check(c.plotDataSections == 1, "basic: one plot data section");
  std::remove(fname.c_str());
}

void testNames(DB& db) {
  string fname = makeTmpName("names");
  db.plot(fname.c_str(), 100.0f, 20.0f, 1.0f, 0.5f, 50.0f,
          /*plotSlacks=*/false, /*plotNets=*/false, /*plotNames=*/true);
  PlotContent c = parsePlotFile(fname);

  // plotNames without plotNets labels only the 3 non-terminal nodes.
  check(c.nameLabels == 3, "names: 3 node name labels");
  check(c.slackLabels == 0, "names: no slack labels");
  check(c.blockRectangles == 3, "names: 3 block rectangles drawn");
  check(c.eofCount == 1, "names: exactly one EOF");
  std::remove(fname.c_str());
}

void testSlacks(DB& db) {
  string fname = makeTmpName("slacks");
  db.plot(fname.c_str(), 100.0f, 20.0f, 1.0f, 0.5f, 50.0f,
          /*plotSlacks=*/true, /*plotNets=*/false, /*plotNames=*/false);
  PlotContent c = parsePlotFile(fname);

  // Two slack labels (x and y) per node, 3 nodes = 6 total.
  check(c.slackLabels == 6, "slacks: 6 slack labels (x and y per node)");
  check(c.nameLabels == 0, "slacks: no name labels");
  check(c.blockRectangles == 3, "slacks: 3 block rectangles drawn");
  check(c.eofCount == 1, "slacks: exactly one EOF");
  std::remove(fname.c_str());
}

void testNets(DB& db) {
  string fname = makeTmpName("nets");
  db.plot(fname.c_str(), 100.0f, 20.0f, 1.0f, 0.5f, 50.0f,
          /*plotSlacks=*/false, /*plotNets=*/true, /*plotNames=*/true);
  PlotContent c = parsePlotFile(fname);

  // plotNets=true adds a second data section for net topology and a second EOF.
  // plotNames=true with plotNets=true also labels the 1 terminal, so 3+1=4
  // labels.
  check(c.nameLabels == 4, "nets: 4 name labels (3 nodes + 1 terminal)");
  check(c.blockRectangles == 3, "nets: 3 block rectangles drawn");
  check(c.eofCount == 2, "nets: two EOFs (block section + net section)");
  check(c.plotDataSections == 2, "nets: two plot data sections");
  std::remove(fname.c_str());
}

void testAll(DB& db) {
  string fname = makeTmpName("all");
  db.plot(fname.c_str(), 100.0f, 20.0f, 1.0f, 0.5f, 50.0f,
          /*plotSlacks=*/true, /*plotNets=*/true, /*plotNames=*/true);
  PlotContent c = parsePlotFile(fname);

  check(c.hasGnuplotHeader, "all: gnuplot header present");
  check(c.hasPauseLine, "all: pause line present");
  check(c.nameLabels == 4, "all: 4 name labels");
  check(c.slackLabels == 6, "all: 6 slack labels");
  check(c.blockRectangles == 3, "all: 3 block rectangles drawn");
  check(c.eofCount == 2, "all: two EOFs");
  check(c.plotDataSections == 2, "all: two plot data sections");
  std::remove(fname.c_str());
}

}  // namespace

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    cerr << "usage: " << argv[0] << " fixture-directory" << endl;
    return 2;
  }

  string fixtureDir(argv[1]);
  std::string fixtureBase = fixtureDir + "/mixed";
  DB db(fixtureBase);

  testBasic(db);
  testNames(db);
  testSlacks(db);
  testNets(db);
  testAll(db);

  if (failures != 0) {
    cerr << failures << " ParquetFP visualization checks failed" << endl;
    return 1;
  }
  cout << "ParquetFP visualization checks passed" << endl;
  return 0;
}
