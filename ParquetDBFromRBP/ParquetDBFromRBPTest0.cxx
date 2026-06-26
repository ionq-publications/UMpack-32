// Row-based placement to Parquet DB conversion driver.
// It loads an RBPlacement, computes the target outline, optionally invokes
// floorplanning, and can save the converted DB design.
// Use it when validating the RBP-to-ParquetDB conversion path.

/**************************************************************************
***
*** Copyright (c) 2000-2006 Regents of the University of Michigan,
***               Saurabh N. Adya, Matt Guthaus and Igor L. Markov
***
***  Contact author(s): sadya@umich.edu, imarkov@umich.edu
***  Original Affiliation:   University of Michigan, EECS Dept.
***                          Ann Arbor, MI 48109-2122 USA
***
***  Permission is hereby granted, free of charge, to any person obtaining
***  a copy of this software and associated documentation files (the
***  "Software"), to deal in the Software without restriction, including
***  without limitation
***  the rights to use, copy, modify, merge, publish, distribute, sublicense,
***  and/or sell copies of the Software, and to permit persons to whom the
***  Software is furnished to do so, subject to the following conditions:
***
***  The above copyright notice and this permission notice shall be included
***  in all copies or substantial portions of the Software.
***
*** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
*** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
*** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
*** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
*** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
*** OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
*** THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***
***
***************************************************************************/
#include <fstream>

#include "ParquetDBFromRBP.h"
#include "RBPlace/RBPlacement.h"
#include "ABKCommon/paramproc.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::vector;

namespace {
bool isRowBasedPlacementAux(const char* auxFileName) {
  ifstream auxFile(auxFileName);
  abkfatal2(auxFile, "Could not open ", auxFileName);

  std::string format;
  auxFile >> format;
  return format == "RowBasedPlacement";
}
}  // namespace

int main(int argc, const char* argv[]) {
  NoParams noParams(argc, argv);
  BoolParam helpRequest("help", argc, argv);
  BoolParam helpRequest1("h", argc, argv);
  BoolParam floorplan("floorplan", argc, argv);
  StringParam saveFile("save", argc, argv);
  StringParam auxFile("f", argc, argv);
  DoubleParam arParam("AR", argc, argv);
  DoubleParam wsParam("WS", argc, argv);
  if (noParams.found() || helpRequest.found() || helpRequest1.found()) {
    cout << "  Use '-help' or '-f filename.aux' " << endl;
    cout << "Options\n"
         << "-AR aspect_ratio      (used when the input is PartProb)\n"
         << "-WS whitespace_pct    (used when the input is PartProb)\n"
         << "-save base_file_name  (save design in floorplan bookshelf "
            "format)\n\n";
    exit(0);
  }

  RBPlacement::Parameters rbParams(argc, argv);

  abkfatal(auxFile.found(), "must have -f <auxfilename>");

  const bool rowBasedAux = isRowBasedPlacementAux(auxFile);
  const double targetAR = arParam.found() ? static_cast<double>(arParam) : 1.0;
  const double targetWS = wsParam.found() ? static_cast<double>(wsParam) : 10.0;
  RBPlacement rbplace =
      rowBasedAux ? RBPlacement(auxFile, rbParams)
                  : RBPlacement(auxFile, targetAR, targetWS, rbParams);
  const HGraphWDimensions& rbHGraph = rbplace.getHGraph();
  double rowHeight = (rbplace.coreRowsBegin())->getHeight();
  double siteSpacing = (rbplace.coreRowsBegin())->getSpacing();
  HGraphParameters hgParams(argc, argv);
  hgParams.makeAllSrcSnk = false;
  HGraphWDimensions hgWDims(auxFile, NULL, NULL, hgParams);
  vector<float> nodeHeights(rbHGraph.getNumNodes(), rowHeight);
  vector<float> nodeWidths(rbHGraph.getNumNodes(),
                           static_cast<float>(siteSpacing));
  for (unsigned i = 0; i < rbHGraph.getNumNodes(); ++i) {
    if (rbHGraph.getWeight(i) > 0) {
      nodeWidths[i] = static_cast<float>(rbHGraph.getWeight(i) / rowHeight);
    }
  }
  HGraphWDimensions parquetHG(rbHGraph, nodeHeights, nodeWidths);
  parquetHG.makeCenterPinOffsets();

  BBox layoutBBox = rbplace.getBBox(false);
  vector<unsigned> nodeIds;
  float nodesArea = hgWDims.getNodesArea();

  for (unsigned i = 0; i < rbHGraph.getNumNodes(); ++i) {
    if (!rbHGraph.isTerminal(i)) {
      nodeIds.push_back(i);
    }
  }
  float reqdArea = layoutBBox.getArea();
  float reqdAR = layoutBBox.getWidth() / layoutBBox.getHeight();
  float maxWS = 100 * (reqdArea - nodesArea) / nodesArea;
  cout << "reqd Aspect Ratio " << reqdAR << endl;
  cout << "reqdArea " << reqdArea << endl;
  cout << "nodesArea " << nodesArea << endl;
  cout << "%WS  " << maxWS << " %" << endl;

  ParquetDBFromRBP* fpDB =
      new ParquetDBFromRBP(rbplace.getPlacement(), parquetHG, nodeIds,
                           layoutBBox, false, false, siteSpacing, rowHeight);
  parquetfp::Command_Line* params = new parquetfp::Command_Line(argc, argv);
  params->reqdAR = reqdAR;
  params->maxWS = maxWS;
  params->scaleTerms = false;
  params->printAnnealerParams();

  /*
  float avgHeight = fpDB->getAvgHeight();
  fpDB->markTallNodesAsMacros(avgHeight);
  fpDB->reduceCoreCellsArea(0.99*reqdArea, 0.15);
  */

  MaxMem maxMem;

  if (floorplan.found()) {
    parquetfp::Annealer annealer(params, fpDB, &maxMem);
    annealer.go();
  }

  /*
  parquetfp::SolveMulti solveMulti(fpDB, params, &maxMem);
  solveMulti.go();
  */
  /*
  vector<Point>& nodeLocs = fpDB->getNodeLocs();
  vector<Orient>& nodeOrients = fpDB->getNodeOrients();
  for(unsigned i=0; i<nodeIds.size(); ++i)
    {
      const HGFNode& node = hgWDims.getNodeByIdx(nodeIds[i]);
      cout<<node.getName()<<" "<<nodeLocs[i]<<" "<<nodeOrients[i]<<endl;
    }
  */

  if (saveFile.found()) {
    fpDB->save(saveFile);
  }

  cout.flush();
  std::cerr.flush();
  std::_Exit(0);
}
