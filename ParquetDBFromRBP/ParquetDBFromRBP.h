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

#ifndef _PARQUETDBFROMRBP_H_
#define _PARQUETDBFROMRBP_H_

#include <cfloat>
#include <map>
#include <vector>

#include "ABKCommon/abktypes.h"
#include "HGraphWDims/hgWDims.h"
#include "ParquetFP/Parquet.h"
#include "Placement/placeWOri.h"

class ParquetDBFromRBP : public parquetfp::DB {
 protected:
  const BBox& _fpRegion;  // BBox of layout region to floorplan
  std::vector<unsigned> _rbpNodes;
  std::map<unsigned, unsigned> _mapping;
  std::vector<Point> _rbpNodesPlacement;
  std::vector<Orient> _rbpNodesOrient;

 private:
 public:
  ParquetDBFromRBP(const PlacementWOrient& pl, const HGraphWDimensions& hg,
                   const std::vector<unsigned>& nodes, const BBox& fpRegion,
                   bool noRotation, bool bloatMacros = false,
                   float siteSpacing = FLT_MAX, float rowSpacing = FLT_MAX);
  ~ParquetDBFromRBP();

  const std::vector<Point>& getNodeLocs();
  const std::vector<Orient>& getNodeOrients();

  // <aaronnn> mark the nodes that require FP
  void markNodesNeedingFP(bit_vector const& nodesNeedingFP);

  // <aaronnn> obstacle handling
  void addObstacles(const PlacementWOrient& pl, const HGraphWDimensions& hg,
                    const std::vector<unsigned>& obstacles, double threshold);
};

#endif
