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
#include "netlist.h"

#include <ABKCommon/abkassert.h>
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#include "basepacking.h"
#include "parsers.h"

using namespace parse_utils;
using namespace basepacking_h;
using std::cout;
using std::endl;
using std::ifstream;
using std::max;
using std::min;
using std::ofstream;
using std::vector;
using std::string;

// --------------------------------------------------------
float NetType::getHPWL(const OrientedPacking& pk) const {
  float minX = Dimension::Infty;
  float maxX = -1 * Dimension::Infty;
  float minY = Dimension::Infty;
  float maxY = -1 * Dimension::Infty;

  for (unsigned int i = 0; i < in_pads.size(); i++) {
    float xloc = in_pads[i].xloc;
    float yloc = in_pads[i].yloc;

    minX = min(minX, xloc);
    maxX = max(maxX, xloc);
    minY = min(minY, yloc);
    maxY = max(maxY, yloc);
    //      printf("PAD [%d]: %lf, %lf\n", i, xloc, yloc);
  }

  for (unsigned int i = 0; i < in_pins.size(); i++) {
    float orig_x_offset = in_pins[i].x_offset;
    float orig_y_offset = in_pins[i].y_offset;

    int blk_index = in_pins[i].block;
    OrientedPacking::ORIENT blk_orient = pk.orient[blk_index];

    float x_length =
        (blk_orient % 2 == 0) ? pk.width[blk_index] : pk.height[blk_index];
    float y_length =
        (blk_orient % 2 == 0) ? pk.height[blk_index] : pk.width[blk_index];

    float x_offset = -1;
    float y_offset = -1;
    getOffsets(orig_x_offset, orig_y_offset, blk_orient, x_offset, y_offset);

    float xloc = pk.xloc[blk_index] + (x_length / 2) + x_offset;
    float yloc = pk.yloc[blk_index] + (y_length / 2) + y_offset;

    minX = min(minX, xloc);
    maxX = max(maxX, xloc);
    minY = min(minY, yloc);
    maxY = max(maxY, yloc);
    //      printf("PIN [%d]: %lf (%lf-%lf), %lf(%lf-%lf)\n",
    //             i, xloc, pk.xloc[blk_index], pk.xloc[blk_index]+x_length,
    //             yloc, pk.yloc[blk_index], pk.yloc[blk_index]+y_length);
  }
  //   printf("x-span: %lf - %lf\n", minX, maxX);
  //   printf("y-span: %lf - %lf\n", minY, maxY);
  //   printf("HPWL: %lf ", (maxX-minX) + (maxY-minY));
  return ((maxX - minX) + (maxY - minY));
}
// --------------------------------------------------------
void NetListType::ParsePl(ifstream& infile,
                          const HardBlockInfoType& blockinfo) {
  in_all_pads.clear();

  ofstream outfile;
  outfile.open("dummy_pl");

  string line;
  int objCount = 0;
  for (int lineCount = 0; getline(infile, line); lineCount++) {
    float xloc, yloc;

    outfile << line << endl;
    std::istringstream lineStream(line.c_str());
    string name;
    if (lineStream >> name >> xloc >> yloc) {
      if (objCount >= blockinfo.blocknum()) {
        PadInfoType temp_pad;
        temp_pad.pad_name = name;
        temp_pad.xloc = xloc;
        temp_pad.yloc = yloc;

        in_all_pads.push_back(temp_pad);
      }
      objCount++;
    }
  }
}
// --------------------------------------------------------
void NetListType::ParseNets(ifstream& infile,
                            const HardBlockInfoType& blockinfo) {
  string line;
  int numNets = -1;
  int numPins = -1;
  abkfatal(infile.good(), "ERROR: .nets file could not be opened successfully");

  // get NumNets
  getline(infile, line);
  while (getline(infile, line)) {
    string firstWord;
    char ch;

    std::istringstream lineStream(line.c_str());
    if ((lineStream >> firstWord >> ch >> numNets) && firstWord == "NumNets")
      break;
  }

  abkfatal(infile.good(), "ERROR in reading .net file (# nets).");

  // get NumPins
  while (getline(infile, line)) {
    string firstWord;
    char ch;

    std::istringstream lineStream(line.c_str());
    if ((lineStream >> firstWord >> ch >> numPins) && firstWord == "NumPins")
      break;
  }

  abkfatal(infile.good(), "ERROR in reading .net file (# pins).");

  // parse the contents of the nets
  int netDegree = -1;  // degree of the current net
  int degreeCount = 0;
  int pinCount = 0;
  vector<NetType::PadType> netPads;  // vector of pads for the current net;
  vector<NetType::PinType> netPins;  // vector of pins for the current net;
  for (int lineCount = 0; infile.good(); lineCount++) {
    getline(infile, line);
    std::istringstream lineStream(line.c_str());
    string firstWord;
    if (!(lineStream >> firstWord)) {
      // blank line
      if (infile.eof()) {
        // wrap up the last net.
        int degreeCount = netPins.size() + netPads.size();
        if (degreeCount == netDegree) {
          NetType temp(netPins, netPads);
          in_nets.push_back(temp);

          degreeCount = 0;
          netPins.clear();
          netPads.clear();
        } else {
          std::stringstream errSS;
          errSS << "ERROR: netDegree in net " << int(in_nets.size())
                << " does not tally";
          abkfatal(0, errSS.str());
        }
      }
    } else if (firstWord[0] == '#') { /* comments (starts with '#') */
    } else if (firstWord == "NetDegree") {
      char colon;
      int number;
      if (lineStream >> colon >> number) {
        // see "NetDegree : x", wrap up previous net
        if (netDegree == -1)
          netDegree = number;
        else if (degreeCount == netDegree) {
          NetType temp(netPins, netPads);
          in_nets.push_back(NetType(netPins, netPads));

          netDegree = number;
          degreeCount = 0;
          netPins.clear();
          netPads.clear();
        } else {
          std::stringstream errSS;
          errSS << "ERROR: netDegree in net " << int(in_nets.size())
                << " does not tally (" << degreeCount << " vs. " << netDegree
                << ").";
          abkfatal(0, errSS.str());
        }
      } else {
        std::stringstream errSS;
        errSS << "ERROR in parsing .net file (line " << lineCount + 1
              << " after NumPins). line: " << line;
        abkfatal(0, errSS.str());
      }
    } else {
      string direction;
      if (lineStream >> direction) {
        string colon;
        string xToken;
        string yToken;
        if (lineStream >> colon >> xToken >> yToken) {
          // consider a pin on a block. Offsets are stored as % values.
          if (colon != ":" || xToken.empty() || yToken.empty() ||
              xToken[0] != '%' || yToken[0] != '%') {
            std::stringstream errSS;
            errSS << "ERROR in parsing .net file (line " << lineCount + 1
                  << " after NumPins). line: " << line;
            abkfatal(0, errSS.str());
          }
          // strtod (not atof) so a non-numeric/overflowing offset is detected
          // rather than silently parsed as 0.
          errno = 0;
          char* xEnd = 0;
          char* yEnd = 0;
          double xVal = std::strtod(xToken.c_str() + 1, &xEnd);
          double yVal = std::strtod(yToken.c_str() + 1, &yEnd);
          if (xEnd == xToken.c_str() + 1 || *xEnd != '\0' ||
              yEnd == yToken.c_str() + 1 || *yEnd != '\0' || errno != 0) {
            std::stringstream errSS;
            errSS << "ERROR in parsing .net file (line " << lineCount + 1
                  << " after NumPins): non-numeric pin offset. line: " << line;
            abkfatal(0, errSS.str());
          }
          float x_percent = static_cast<float>(xVal);
          float y_percent = static_cast<float>(yVal);
          int index = get_index(firstWord, blockinfo);
          if (index == -1) {
            std::stringstream errSS;
            errSS << "ERROR: Unrecognized block \"" << firstWord << "\"";
            abkfatal(0, errSS.str());
          }
          x_percent /= 100;
          y_percent /= 100;

          NetType::PinType temp;
          temp.block = index;
          temp.x_offset = (blockinfo[index].width[0] * x_percent) / 2;
          temp.y_offset = (blockinfo[index].height[0] * y_percent) / 2;
          netPins.push_back(temp);
          degreeCount++;
          pinCount++;
        } else {
          // consider a pad with name "firstWord".
          vector<int> indices;
          get_index(firstWord, indices);
          if (indices.empty()) {
            std::stringstream errSS;
            errSS << "ERROR: unrecognized pad \"" << firstWord << "\"";
            abkfatal(0, errSS.str());
          }
          for (unsigned int i = 0; i < indices.size(); i++) {
            NetType::PadType temp;
            temp.xloc = in_all_pads[indices[i]].xloc;
            temp.yloc = in_all_pads[indices[i]].yloc;
            netPads.push_back(temp);
            pinCount++;
          }
          degreeCount++;
        }
      } else {
        std::stringstream errSS;
        errSS << "ERROR in parsing .net file (line " << lineCount + 1
              << " after NumPins). line: " << line;
        abkfatal(0, errSS.str());
      }
    }
  }

  if (pinCount != numPins) {
    std::stringstream errSS;
    errSS << "ERROR: # pins does not tally (" << pinCount << " vs. " << numPins
          << ").";
    abkfatal(0, errSS.str());
  } else if (int(in_nets.size()) != numNets) {
    std::stringstream errSS;
    errSS << "ERROR: # nets does not tally (" << int(in_nets.size()) << " vs. "
          << numNets << ").";
    abkfatal(0, errSS.str());
  }
  infile.close();
}
// --------------------------------------------------------
