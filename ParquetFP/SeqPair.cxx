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

#include "SeqPair.h"

#include <algorithm>

#include "FPcommon.h"

using namespace parquetfp;
using std::cout;
using std::endl;
using std::vector;

SeqPair::SeqPair(unsigned size)  // ctor randomly generates the seqPair
{
  _XX.resize(size);
  _YY.resize(size);
  for (unsigned i = 0; i < size; ++i) {
    _XX[i] = i;
    _YY[i] = i;
  }
  for (vector<unsigned>::iterator i = _XX.end(); i != _XX.begin(); --i) {
    vector<unsigned>::iterator target =
        _XX.begin() + (rand() % (i - _XX.begin()));
    std::iter_swap(i - 1, target);
  }
  for (vector<unsigned>::iterator i = _YY.end(); i != _YY.begin(); --i) {
    vector<unsigned>::iterator target =
        _YY.begin() + (rand() % (i - _YY.begin()));
    std::iter_swap(i - 1, target);
  }
}

SeqPair::SeqPair(const vector<unsigned>& X, const vector<unsigned>& Y)
    : _XX(X), _YY(Y) {
  if (X.size() != Y.size())
    cout << "ERROR: Input Sequence Pairs of different sizes" << endl;
}
