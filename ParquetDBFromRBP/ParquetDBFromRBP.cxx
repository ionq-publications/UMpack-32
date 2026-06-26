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

#include "ParquetDBFromRBP.h"

#include <cmath>
#include <sstream>
#include <string>

#include "ABKCommon/abkassert.h"
using std::cout;
using std::endl;
using std::map;
using std::ostringstream;
using std::string;
using std::vector;

// Conversion helper used by the ParquetDBFromRBP test driver.
// It turns an RBPlacement plus hypergraph into a Parquet floorplanning DB
// and preserves the row-based placement information for later floorplanning.

#define SKIP_NETS_LARGER_THAN 200

namespace {
string fallbackNodeName(unsigned index, char prefix) {
  ostringstream name;
  name << prefix << index;
  return name.str();
}

string safeNodeName(const HGraphWDimensions&, unsigned index, char prefix) {
  return fallbackNodeName(index, prefix);
}

void sanitizeNodeDims(const HGraphWDimensions& hg, unsigned index,
                      float rowSpacing, float& nodeWidth,
                      float& nodeHeight) {
  nodeWidth = static_cast<float>(hg.getNodeWidth(index));
  nodeHeight = static_cast<float>(hg.getNodeHeight(index));

  if (!(std::isfinite(nodeWidth) && nodeWidth > 0 && std::isfinite(nodeHeight) &&
        nodeHeight > 0) &&
      std::isfinite(rowSpacing) && rowSpacing > 0 &&
      std::isfinite(hg.getWeight(index)) && hg.getWeight(index) > 0) {
    nodeHeight = rowSpacing;
    nodeWidth = static_cast<float>(hg.getWeight(index) / rowSpacing);
  }

  if (!(std::isfinite(nodeWidth) && nodeWidth > 0)) nodeWidth = 1.0f;
  if (!(std::isfinite(nodeHeight) && nodeHeight > 0)) nodeHeight = 1.0f;
}

BBox centerPinBBox(float nodeWidth, float nodeHeight) {
  BBox pinBBox;
  const double x = nodeWidth / 2.0;
  const double y = nodeHeight / 2.0;
  pinBBox.add(x, y);
  pinBBox.add(x, y);
  return pinBBox;
}
}  // namespace

ParquetDBFromRBP::~ParquetDBFromRBP() {
  _mapping.clear();
  _rbpNodesPlacement.clear();
  _rbpNodesOrient.clear();
}

ParquetDBFromRBP::ParquetDBFromRBP(const PlacementWOrient& pl,
                                   const HGraphWDimensions& hg,
                                   const vector<unsigned>& nodes,
                                   const BBox& fpRegion, bool noRotation,
                                   bool bloatMacros, float siteSpacing,
                                   float rowSpacing)
    : parquetfp::DB(), _fpRegion(fpRegion), _rbpNodes(nodes) {
  _rbpNodesPlacement.resize(_rbpNodes.size());
  _rbpNodesOrient.resize(_rbpNodes.size());

  unsigned i;
  unsigned index;
  float nodesArea = 0;

  vector<bool> instanceNodes(hg.getNumNodes(), false);
  vector<bool> seenNodes(hg.getNumNodes(), false);
  map<unsigned, unsigned> mapTerminals;

  for (i = 0; i < _rbpNodes.size(); ++i) {
    // index: the actual node index
    // i: the index of that node in this DB
    // mapping: index --> i (the inverse is captured by _rbpNodes)
    index = _rbpNodes[i];
    const HGFNode& origNode = hg.getNodeByIdx(index);
    float nodeWidth, nodeHeight;
    sanitizeNodeDims(hg, index, rowSpacing, nodeWidth, nodeHeight);

    bool nodeIsSoft = hg.isNodeSoft(index);

    if (bloatMacros && hg.isNodeMacro(index) && !nodeIsSoft) {
      nodeWidth = ceil(nodeWidth / siteSpacing) * siteSpacing;
      nodeHeight = ceil(nodeHeight / rowSpacing) * rowSpacing;
    }

    float nodeArea = nodeWidth * nodeHeight;

    float nodeMaxAR = (nodeIsSoft) ? static_cast<float>(hg.getNodeMaxAR(index))
                                   : nodeWidth / nodeHeight;
    float nodeMinAR = (nodeIsSoft) ? static_cast<float>(hg.getNodeMinAR(index))
                                   : nodeWidth / nodeHeight;

    parquetfp::Node tempNode(safeNodeName(hg, origNode.getIndex(), 'n'),
                             nodeArea, nodeMinAR, nodeMaxAR, i, false);
    /*
    if(!hg.isNodeMacro(index))
      tempNode.putIsOrientFixed(true);
        */

    if (noRotation && hg.isNodeMacro(index)) tempNode.putIsOrientFixed(true);

    tempNode.addSubBlockIndex(i);
    tempNode.putX(static_cast<float>(pl[index].x));
    tempNode.putY(static_cast<float>(pl[index].y));
    _nodes->putNewNode(tempNode);
    _mapping[index] = i;
    _rbpNodesPlacement[i] = pl[index];
    nodesArea += nodeArea;
    instanceNodes[index] = true;
  }

  float termXLoc = 0.f, termYLoc = 0.f;
  float minXLoc = static_cast<float>(_fpRegion.xMin);
  float maxXLoc = static_cast<float>(_fpRegion.xMax);
  float minYLoc = static_cast<float>(_fpRegion.yMin);
  float maxYLoc = static_cast<float>(_fpRegion.xMax);

  float reqdHeight = static_cast<float>(_fpRegion.getHeight());
  float reqdWidth = static_cast<float>(_fpRegion.getWidth());
  // float reqdArea = reqdHeight*reqdWidth;
  // float reqdAR = reqdWidth/reqdHeight;
  // float maxWS = 100*(reqdArea-nodesArea)/nodesArea;

  unsigned numOrigNets = hg.getNumEdges();
  vector<bool> seenNets(numOrigNets, false);

  unsigned newNetCtr = 0;
  unsigned newTermCtr = 0;

  itHGFEdgeLocal origEdge;
  itHGFNodeLocal n;
  parquetfp::Net tempEdge;

  for (i = 0; i < _rbpNodes.size(); ++i) {
    index = _rbpNodes[i];
    const HGFNode& origNode = hg.getNodeByIdx(index);
    for (origEdge = origNode.edgesBegin(); origEdge != origNode.edgesEnd();
         ++origEdge) {
      unsigned origNetIdx = (*origEdge)->getIndex();

      if ((*origEdge)->getDegree() > SKIP_NETS_LARGER_THAN)
        seenNets[origNetIdx] = true;

      if (!seenNets[origNetIdx]) {
        seenNets[origNetIdx] = true;

        unsigned nodeOffset = 0;
        BBox netBBox;
        for (n = (*origEdge)->nodesBegin(); n != (*origEdge)->nodesEnd();
             n++, nodeOffset++) {
          unsigned cellId = (*n)->getIndex();
          float cellWidth, cellHeight;
          sanitizeNodeDims(hg, cellId, rowSpacing, cellWidth, cellHeight);
          BBox pinLoc = centerPinBBox(cellWidth, cellHeight);

          pinLoc.TranslateBy(pl[cellId]);
          netBBox.expandToInclude(pinLoc);
        }

        // remove this inessential net for faster fping
        if (netBBox.isEmpty() || netBBox.contains(fpRegion)) continue;

        tempEdge.putIndex(newNetCtr);
        tempEdge.putWeight(static_cast<float>((*origEdge)->getWeight()));

        nodeOffset = 0;
        for (n = (*origEdge)->nodesBegin(); n != (*origEdge)->nodesEnd();
             n++, nodeOffset++) {
          unsigned cellId = (*n)->getIndex();
          //               Point cellPl = pl[cellId];

          float cellWidth, cellHeight;
          sanitizeNodeDims(hg, cellId, rowSpacing, cellWidth, cellHeight);
          BBox pinOffset = centerPinBBox(cellWidth, cellHeight);

          //               BBox pinLoc = pinOffset;
          //               pinLoc.TranslateBy(pl[cellId]);

          float poffsetXmin, poffsetYmin, poffsetXmax, poffsetYmax;
          poffsetXmin = static_cast<float>((pinOffset.xMin / cellWidth) - 0.5);
          poffsetXmax = static_cast<float>((pinOffset.xMax / cellWidth) - 0.5);
          poffsetYmin = static_cast<float>((pinOffset.yMin / cellHeight) - 0.5);
          poffsetYmax = static_cast<float>((pinOffset.yMax / cellHeight) - 0.5);

          if (instanceNodes[cellId]) {
            unsigned newNodeIdx = _mapping[cellId];

            parquetfp::Node& newNode = _nodes->getNode(newNodeIdx);
            parquetfp::pin *pin1 = 0, *pin2 = 0;
            pin1 = new parquetfp::pin(newNode.getName(), false, poffsetXmin,
                                      poffsetYmin, newNetCtr);
            if (poffsetXmin != poffsetXmax || poffsetYmin != poffsetYmax)
              pin2 = new parquetfp::pin(newNode.getName(), false, poffsetXmax,
                                        poffsetYmax, newNetCtr);

            pin1->putNodeIndex(newNodeIdx);
            tempEdge.addNode(*pin1);
            delete pin1;
            if (pin2) {
              pin2->putNodeIndex(newNodeIdx);
              tempEdge.addNode(*pin2);
              delete pin2;
            }

          } else  // need terminal propagation
          {
            unsigned termId;
            if (!seenNodes[cellId]) {
              string tmpNm = safeNodeName(hg, (*n)->getIndex(), 't');
              parquetfp::Node tempTerm(tmpNm, 0, 1, 1, newTermCtr, true);
              tempTerm.addSubBlockIndex(newTermCtr);

              BBox pinLoc = centerPinBBox(cellWidth, cellHeight);
              pinLoc.TranslateBy(pl[cellId]);

              // terminal propagation
              if (static_cast<float>(pinLoc.xMin) < minXLoc)
                termXLoc = 0.f;
              else if (static_cast<float>(pinLoc.xMax) > maxXLoc)
                termXLoc = reqdWidth;
              else
                termXLoc =
                    static_cast<float>(pinLoc.getGeomCenter().x) - minXLoc;

              if (static_cast<float>(pinLoc.yMin) < minYLoc)
                termYLoc = 0.f;
              else if (static_cast<float>(pinLoc.yMax) > maxYLoc)
                termYLoc = reqdHeight;
              else
                termYLoc =
                    static_cast<float>(pinLoc.getGeomCenter().y) - minYLoc;

              tempTerm.putX(termXLoc);
              tempTerm.putY(termYLoc);

              _nodes->putNewTerm(tempTerm);
              mapTerminals[cellId] = newTermCtr;
              termId = newTermCtr;
              newTermCtr++;
            } else {
              termId = mapTerminals[cellId];
            }
            parquetfp::pin tempPin(safeNodeName(hg, (*n)->getIndex(), 't'),
                                   true, 0, 0, termId);
            tempPin.putNodeIndex(termId);
            tempEdge.addNode(tempPin);
          }
          seenNodes[cellId] = true;
        }
        _nets->putNewNet(tempEdge);
        ++newNetCtr;
        tempEdge.clean();
      }
    }
  }
  _nodes->updatePinsInfo(*_nets);
  _nodes->initNodesFastPOAccess(*_nets, false);

  *_nodesBestCopy = *_nodes;

  buildTermBBox();
}

const vector<Point>& ParquetDBFromRBP::getNodeLocs() {
  unsigned numNodes = _rbpNodesPlacement.size();
  for (unsigned i = 0; i < numNodes; ++i) {
    const parquetfp::Node& node = _nodes->getNode(i);
    _rbpNodesPlacement[i].x = static_cast<double>(node.getX());
    _rbpNodesPlacement[i].y = static_cast<double>(node.getY());
  }
  return _rbpNodesPlacement;
}

const vector<Orient>& ParquetDBFromRBP::getNodeOrients() {
  unsigned numNodes = _rbpNodesOrient.size();
  for (unsigned i = 0; i < numNodes; ++i) {
    const parquetfp::Node& node = _nodes->getNode(i);
    const parquetfp::ORIENT nodeOrient = node.getOrient();

    if (nodeOrient == parquetfp::N)
      _rbpNodesOrient[i] = Orient("N");
    else if (nodeOrient == parquetfp::S)
      _rbpNodesOrient[i] = Orient("S");
    else if (nodeOrient == parquetfp::E)
      _rbpNodesOrient[i] = Orient("E");
    else if (nodeOrient == parquetfp::W)
      _rbpNodesOrient[i] = Orient("W");
    else if (nodeOrient == parquetfp::FN)
      _rbpNodesOrient[i] = Orient("FN");
    else if (nodeOrient == parquetfp::FS)
      _rbpNodesOrient[i] = Orient("FS");
    else if (nodeOrient == parquetfp::FE)
      _rbpNodesOrient[i] = Orient("FE");
    else if (nodeOrient == parquetfp::FW)
      _rbpNodesOrient[i] = Orient("FW");
    else {
      cout << "Something wrong with node " << i << endl;
      abkfatal(0, "Invalid Orientation returned\n");
    }
  }
  return _rbpNodesOrient;
}

void ParquetDBFromRBP::markNodesNeedingFP(bit_vector const& nodesNeedingFP) {
  for (unsigned i = 0; i < nodesNeedingFP.size(); i++) {
    if (nodesNeedingFP[i] == false) continue;

    unsigned capoIdx = i;

    if (_mapping.find(capoIdx) == _mapping.end()) continue;

    unsigned DBIdx = _mapping[capoIdx];
    parquetfp::Node& node = _nodes->getNode(DBIdx);
    node.putNeedsFP(true);
    // cout << "ANDBG node " << DBIdx << " (" << node.getArea() << ") , " <<
    // capoIdx << " needs FP " << endl;
  }
}

void ParquetDBFromRBP::addObstacles(const PlacementWOrient& pl,
                                    const HGraphWDimensions& hg,
                                    const vector<unsigned>& obstacles,
                                    double threshold)
/*
 * <aaronnn> pass a vector of obstacles to DB. This function is the conduit
 * that passes Capo's obstacle-node information to DB, and does whatever
 * necessary packaging/processing
 */
{
  // in the floorplanner, we only care about the obstacles' dimensions and
  // locations. Nets connecting to obstacles will be handled by the current
  // terminal propagation code.
  // cout << "ANDBG fpRegion " << _fpRegion.xMin << " " << _fpRegion.yMin <<
  // endl;

  double binArea = _fpRegion.getArea();
  double locX, locY;
  for (unsigned i = 0; i < obstacles.size(); ++i) {
    unsigned index = obstacles[i];
    const HGFNode& origNode = hg.getNodeByIdx(index);

    unsigned angle = pl.getOrient(index).getAngle();
    bool notRotated = angle == 0 || angle == 180;
    float nodeWidth = notRotated ? static_cast<float>(hg.getNodeWidth(index))
                                 : static_cast<float>(hg.getNodeHeight(index));
    float nodeHeight = notRotated ? static_cast<float>(hg.getNodeHeight(index))
                                  : static_cast<float>(hg.getNodeWidth(index));

    BBox nodeBBox;
    nodeBBox.add(pl[index].x, pl[index].y);
    nodeBBox.add(pl[index].x + nodeWidth, pl[index].y + nodeHeight);

    BBox intersection = nodeBBox.intersect(_fpRegion);
    nodeWidth = static_cast<float>(intersection.getWidth());
    nodeHeight = static_cast<float>(intersection.getHeight());
    locX = intersection.xMin;
    locY = intersection.yMin;

    float nodeAR = nodeWidth / nodeHeight;
    float nodeArea = static_cast<float>(intersection.getArea());

    parquetfp::Node obstacleNode(safeNodeName(hg, origNode.getIndex(), 'o'),
                                 nodeArea, nodeAR, nodeAR, i, false);

    if (nodeArea > threshold * binArea) {
      obstacleNode.putIsOrientFixed(true);
      obstacleNode.addSubBlockIndex(i);

      // NOTE: obstacles have x,y location relative to bin's bottom-left
      // because, ParquetDBFromRBP is the only derived class that knows about
      // _fpRegion, and the obstacles matter in the annealer, which doesn't
      // really want absolute locations.

      obstacleNode.putX(static_cast<float>(locX - _fpRegion.xMin));
      obstacleNode.putY(static_cast<float>(locY - _fpRegion.yMin));
      // cout << "ANDBG adding obstacle " << obstacleNode.getX() << " " <<
      // obstacleNode.getY()
      //     << " " << nodeWidth << " " << nodeHeight << endl;

      _obstacles->putNewNode(obstacleNode);
    }
  }

  _obstacleFrame[0] = _fpRegion.getWidth();
  _obstacleFrame[1] = _fpRegion.getHeight();
}
