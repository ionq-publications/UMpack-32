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

#include "basepacking.h"

#include <ABKCommon/abkassert.h>
#include <ABKCommon/abklimits.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "parsers.h"
using namespace parse_utils;
using namespace basepacking_h;
using std::cout;
using std::endl;
using std::ifstream;
using std::vector;
using std::string;

// ======================
// Contructors in the end
// ======================
const float Dimension::Infty = std::numeric_limits<float>::max();
const float Dimension::Epsilon_Accuracy = 1e10f;
const int Dimension::Undefined = -1;
const int Dimension::Orient_Num = 8;

const int HardBlockInfoType::Orient_Num = Dimension::Orient_Num;
// ========================================================
void HardBlockInfoType::set_dimensions(int i, float w, float h) {
  in_blocks[i].width.resize(Orient_Num);
  in_blocks[i].height.resize(Orient_Num);
  for (int j = 0; j < Orient_Num; j++)
    if (j % 2 == 0) {
      in_blocks[i].width[j] = w;
      in_blocks[i].height[j] = h;
    } else {
      in_blocks[i].width[j] = h;
      in_blocks[i].height[j] = w;
    }
}
// --------------------------------------------------------
HardBlockInfoType::HardBlockInfoType(ifstream& ins,
                                     const std::string& format)
    : blocks(in_blocks), block_names(in_block_names) {
  if (format == "txt")
    ParseTxt(ins);
  else if (format == "blocks")
    ParseBlocks(ins);
  else {
    std::stringstream errSS;
    errSS << "ERROR: invalid format: " << format;
    abkfatal(0, errSS.str());
  }
}
// --------------------------------------------------------
void HardBlockInfoType::ParseTxt(ifstream& ins) {
  int blocknum = -1;
  ins >> blocknum;

  abkfatal(ins.good(), "ERROR: cannot read the block count.");

  if (blocknum < 0) {
    std::stringstream errSS;
    errSS << "ERROR: negative block count: " << blocknum;
    abkfatal(0, errSS.str());
  }

  // Compute the size in size_t to avoid signed int overflow in (blocknum + 2);
  // an absurdly large count then fails in resize() rather than wrapping.
  in_blocks.resize(static_cast<size_t>(blocknum) + 2);
  in_block_names.resize(static_cast<size_t>(blocknum) + 2);
  for (int i = 0; i < blocknum; i++) {
    float w, h;
    ins >> w >> h;

    if (!ins.good()) {
      std::stringstream errSS;
      errSS << "ERROR: cannot read block no." << i;
      abkfatal(0, errSS.str());
    }

    set_dimensions(i, w, h);

    std::stringstream ss;
    ss << i;
    in_block_names[i] = ss.str();
  }
  set_dimensions(blocknum, 0, Dimension::Infty);
  in_block_names[blocknum] = "LEFT";

  set_dimensions(blocknum + 1, Dimension::Infty, 0);
  in_block_names[blocknum + 1] = "BOTTOM";
}
// --------------------------------------------------------
// taken from Nodes.cxx of Parquet using FPcommon.h/cxx
// --------------------------------------------------------
void HardBlockInfoType::ParseBlocks(ifstream& ins) {
  std::string blockName;
  std::string blockType;
  std::string tempWord;

  int numSoftBl = 0;
  int numHardBl = 0;
  int numTerm = 0;

  int indexBlock = 0;

  abkfatal(ins, "ERROR: .blocks file could not be opened successfully");

  skiptoeol(ins);
  while (!ins.eof()) {
    ins >> tempWord;
    if (tempWord == "NumSoftRectangularBlocks") break;
  }

  abkfatal(ins.good(), "ERROR in parsing .blocks file.");

  ins >> tempWord;
  ins >> numSoftBl;
  if (numSoftBl != 0) {
    cout << "ERROR: soft block packing is not supported for now." << endl;
    exit(0);
  }

  while (!ins.eof()) {
    ins >> tempWord;
    if (tempWord == "NumHardRectilinearBlocks") break;
  }
  ins >> tempWord;
  ins >> numHardBl;

  while (!ins.eof()) {
    ins >> tempWord;
    if (tempWord == "NumTerminals") break;
  }
  ins >> tempWord;
  ins >> numTerm;

  in_blocks.resize(numHardBl + 2);
  in_block_names.resize(numHardBl + 2);
  while (ins.good()) {
    blockType.clear();
    eatblank(ins);

    if (ins.eof()) break;
    if (ins.peek() == '#')
      eathash(ins);
    else {
      eatblank(ins);
      if (ins.peek() == '\n' || ins.peek() == '\r') {
        ins.get();
        continue;
      }

      ins >> blockName;
      ins >> blockType;

      if (blockType == "softrectangular") {
        cout << "ERROR: soft block packing is not supported now." << endl;
        exit(1);
      } else if (blockType == "hardrectilinear") {
        Point tempPoint;
        vector<Point> vertices;
        int numVertices;
        bool success;
        float width, height;

        ins >> numVertices;
        success = true;
        abkfatal(numVertices == 4,
                 "ERROR in parsing .blocks file. rectilinear blocks can be "
                 "only rectangles for now");

        for (int i = 0; i < numVertices; ++i) {
          success &= needCaseChar(ins, '(');
          ins.get();
          ins >> tempPoint.x;
          success &= needCaseChar(ins, ',');
          ins.get();
          ins >> tempPoint.y;
          success &= needCaseChar(ins, ')');
          ins.get();
          vertices.push_back(tempPoint);
        }
        abkfatal(success,
                 "ERROR in parsing .blocks file while processing "
                 "hardrectilinear blocks.");

        width = vertices[2].x - vertices[0].x;
        height = vertices[2].y - vertices[0].y;

        // consider a block
        //             cout << "[" << indexBlock << "] "
        //                  << setw(10) << block_name
        //                  << " width: " << width
        //                  << " height: " << height << endl;
        // Bounds-check before writing: set_dimensions() indexes in_blocks
        // unconditionally, so a .blocks file with more hardrectilinear entries
        // than its declared NumHardRectilinearBlocks would otherwise write past
        // the end of the vector.
        abkfatal(indexBlock < int(block_names.size()),
                 "ERROR: too many hard blocks specified.");
        set_dimensions(indexBlock, width, height);
        in_block_names[indexBlock] = blockName;
        ++indexBlock;
      } else if (blockType == "terminal") { /* a pad */
      } else if (ins.good()) {
        std::stringstream errSS;
        errSS << "ERROR: invalid block type: " << blockType;
        abkfatal(0, errSS.str());
      }
    }
  }  // end of while-loop
  ins.close();

  if (numSoftBl + numHardBl != indexBlock) {
    std::stringstream errSS;
    errSS << "ERROR in parsing .blocks file. # blocks do not tally "
          << indexBlock << " vs. " << (numSoftBl + numHardBl);
    abkfatal(0, errSS.str());
  }
  set_dimensions(numHardBl, 0, Dimension::Infty);
  in_block_names[numHardBl] = "LEFT";

  set_dimensions(numHardBl + 1, Dimension::Infty, 0);
  in_block_names[numHardBl + 1] = "BOTTOM";
}
// ========================================================
