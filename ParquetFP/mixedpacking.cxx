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

#include "mixedpacking.h"

#include <ABKCommon/abkassert.h>

#include <sstream>

#include <cmath>
#include <fstream>
#include <string>

#include "basepacking.h"
#include "parsers.h"
using namespace parse_utils;
using namespace basepacking_h;
using std::cout;
using std::endl;
using std::max;
using std::min;
using std::vector;
using std::string;

const int MixedBlockInfoType::Orient_Num = HardBlockInfoType::Orient_Num;
// --------------------------------------------------------
MixedBlockInfoType::MixedBlockInfoType(const std::string& blocksfilename,
                                       const std::string& format)
    : currDimensions(_currDimensions),
      blockARinfo(_blockARinfo),
      _currDimensions(0) {
  std::ifstream infile;
  infile.open(blocksfilename.c_str());
  if (!infile.good()) {
    abkfatal2(0, "ERROR: cannot open file ", blocksfilename);
  }

  if (format == "txt") {
    cout << "Sorry, .txt format isn't supported now." << endl;
    exit(0);
  } else if (format == "blocks")
    ParseBlocks(infile);
}
// --------------------------------------------------------
void MixedBlockInfoType::ParseBlocks(std::ifstream& input) {
  string blockName;
  string blockType;
  string tempWord1;

  vector<Point> vertices;
  int numVertices;
  bool success;
  float width, height;

  float area, minAr, maxAr;
  int numSoftBl = 0;
  int numHardBl = 0;
  int numBl = numSoftBl + numHardBl;
  int numTerm = 0;

  int indexBlock = 0;
  int indexTerm = 0;

  if (!input) {
    cout << "ERROR: .blocks file could not be opened successfully" << endl;
    exit(0);
  }

  while (!input.eof()) {
    input >> tempWord1;
    if (tempWord1 == "NumSoftRectangularBlocks") break;
  }
  input >> tempWord1;
  input >> numSoftBl;

  while (!input.eof()) {
    input >> tempWord1;
    if (tempWord1 == "NumHardRectilinearBlocks") break;
  }
  input >> tempWord1;
  input >> numHardBl;

  while (!input.eof()) {
    input >> tempWord1;
    if (tempWord1 == "NumTerminals") break;
  }
  input >> tempWord1;
  input >> numTerm;

  numBl = numHardBl + numSoftBl;
  _currDimensions.in_blocks.resize(numBl + 2);
  _currDimensions.in_block_names.resize(numBl + 2);
  _blockARinfo.resize(numHardBl + numSoftBl + 2);
  while (!input.eof()) {
    blockType.clear();
    eatblank(input);
    if (input.eof()) break;
    if (input.peek() == '#')
      eathash(input);
    else {
      eatblank(input);
      if (input.peek() == '\n' || input.peek() == '\r') {
        input.get();
        continue;
      }

      input >> blockName;
      input >> blockType;

      if (blockType == "softrectangular") {
        input >> area;
        input >> minAr;
        input >> maxAr;

        width = sqrt(area);
        height = sqrt(area);

        //             printf("[%d]: area: %.2lf minAR: %.2lf maxAR: %.2lf
        //             width: %.2lf height: %.2lf\n",
        //                    indexBlock, area, minAr, maxAr, width, height);
        if (indexBlock >= int(_currDimensions.block_names.size())) {
          abkfatal(0, "ERROR: too many blocks specified.");
        }
        _currDimensions.set_dimensions(indexBlock, width, height);
        _currDimensions.in_block_names[indexBlock] = blockName;

        _blockARinfo[indexBlock].area = area;
        set_blockARinfo_AR(indexBlock, min(minAr, maxAr), max(minAr, maxAr));
        _blockARinfo[indexBlock].isSoft = true;

        ++indexBlock;
        // cout<<block_name<<" "<<area<<endl;
      } else if (blockType == "hardrectilinear") {
        input >> numVertices;
        Point tempPoint;
        success = 1;
        if (numVertices != 4) {
          abkfatal(0,
                   "ERROR in parsing .blocks file. rectilinear blocks can be "
                   "only rectangles for now");
        }
        for (int i = 0; i < numVertices; ++i) {
          success &= needCaseChar(input, '(');
          input.get();
          input >> tempPoint.x;
          success &= needCaseChar(input, ',');
          input.get();
          input >> tempPoint.y;
          success &= needCaseChar(input, ')');
          input.get();
          vertices.push_back(tempPoint);
        }
        abkfatal(success,
                 "ERROR in parsing .blocks file while processing "
                 "hardrectilinear blocks");

        width = vertices[2].x - vertices[0].x;
        height = vertices[2].y - vertices[0].y;
        area = width * height;
        minAr = width / height;
        maxAr = minAr;

        if (indexBlock >= int(_currDimensions.block_names.size())) {
          abkfatal(0, "ERROR: too many hard blocks specified.");
        }
        _currDimensions.set_dimensions(indexBlock, width, height);
        _currDimensions.in_block_names[indexBlock] = blockName;

        _blockARinfo[indexBlock].area = area;
        set_blockARinfo_AR(indexBlock, min(minAr, maxAr), max(minAr, maxAr));
        _blockARinfo[indexBlock].isSoft = false;

        ++indexBlock;
        vertices.clear();
        // cout<<block_name<<" "<<area<<endl;
      } else if (blockType == "terminal") {
        ++indexTerm;
        // cout<<indexTerm<<"  "<<block_name<<endl;
      }
      /*
        else
        cout<<"ERROR in parsing .blocks file"<<endl;
      */
    }
  }
  input.close();

  if (numSoftBl + numHardBl != indexBlock) {
    std::stringstream errSS;
    errSS << "ERROR in parsing .blocks file. Number of blocks does not tally "
          << (indexBlock + indexTerm) << " vs "
          << (numSoftBl + numHardBl + numTerm);
    abkfatal(0, errSS.str());
  }

  _currDimensions.set_dimensions(numBl, 0, Dimension::Infty);
  _currDimensions.in_block_names[numBl] = "LEFT";
  _blockARinfo[numBl].area = 0;
  _blockARinfo[numBl].minAR.resize(Orient_Num, 0);
  _blockARinfo[numBl].maxAR.resize(Orient_Num, 0);
  _blockARinfo[numBl].isSoft = false;

  _currDimensions.set_dimensions(numBl + 1, Dimension::Infty, 0);
  _currDimensions.in_block_names[numBl + 1] = "BOTTOM";
  _blockARinfo[numBl + 1].area = 0;
  _blockARinfo[numBl + 1].minAR.resize(Orient_Num, Dimension::Infty);
  _blockARinfo[numBl + 1].maxAR.resize(Orient_Num, Dimension::Infty);
  _blockARinfo[numBl + 1].isSoft = false;
}
// --------------------------------------------------------
