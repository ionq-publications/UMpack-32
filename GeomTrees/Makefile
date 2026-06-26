#/**************************************************************************
#***    
#*** Copyright (c) 1995-2000 Regents of the University of California,
#***               Andrew E. Caldwell, Andrew B. Kahng and Igor L. Markov
#*** Copyright (c) 2000-2012 Regents of the University of Michigan,
#***               Saurabh N. Adya, Jarrod A. Roy, David A. Papa and
#***               Igor L. Markov
#***
#***  Contact author(s): abk@cs.ucsd.edu, imarkov@umich.edu
#***  Original Affiliation:   UCLA, Computer Science Department,
#***                          Los Angeles, CA 90095-1596 USA
#***
#***  Permission is hereby granted, free of charge, to any person obtaining 
#***  a copy of this software and associated documentation files (the
#***  "Software"), to deal in the Software without restriction, including
#***  without limitation 
#***  the rights to use, copy, modify, merge, publish, distribute, sublicense, 
#***  and/or sell copies of the Software, and to permit persons to whom the 
#***  Software is furnished to do so, subject to the following conditions:
#***
#***  The above copyright notice and this permission notice shall be included
#***  in all copies or substantial portions of the Software.
#***
#*** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
#*** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#*** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
#*** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
#*** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
#*** OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
#*** THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#***
#***
#***************************************************************************/



# 970801 Created and tained by Igor Markov,  VLSICAD ABKGroup, UCLA/CS
# 970814 ilm added ir.out to clean, added +p switch, changed DEBUG to ABKDEBUG

.SUFFIXES:	.o .cxx .exe .exeS .h.gch

MAKE     = make
 OPTFLAGS = -g -DABKDEBUG
#OPTFLAGS= -O3 # use this for optimization instead of the above
CCFLAGS          = -fPIC
DEPENDOPT= -xM1
CC               = g++ $(OPTFLAGS) $(CCFLAGS)
CC4LIBS          = g++
LD               = g++ -shared
LDFINAL          = g++ -Wl,--no-as-needed
AR               = ar cr
SODIRKEY         = -Wl,-rpath,
SONAMEKEY        = -Wl,-soname,
                      # LD_LIBRARY_PATH overrides -h
UCLA     := $(abspath $(dir $(abspath $(lastword $(MAKEFILE_LIST))))/..)
IMPORT   = NOT_USED
LIBBASE  := $(UCLA)/lib
SLIBBASE         = $(LIBBASE)
ARNAME      = lib$(THISLIB).a
SONAME      = lib$(THISLIB).so
SODIR            = $(CURDIR)
SOFULL           = $(SODIR)/$(SONAME)
SOLABEL          = $(SONAMEKEY)$(SONAME)
TESTNAME    = $(THISLIB)Test

STL         = 
# -I$(IMPORT)/STL/CC

# #########################################################
# standard variables that need modification in each project
# #########################################################

THISLIB = GeomTrees
INCLUDES= $(STL) -I. -I$(UCLA)
FSOBJS  = FastSteiner/unixtimer.o FastSteiner/err.o \
          FastSteiner/neighbors.o FastSteiner/heap.o FastSteiner/mst2.o \
          FastSteiner/stnr1.o FastSteiner/triples.o FastSteiner/umextra.o
FSSRCS  = $(FSOBJS:%.o=%.cxx)
OBJS    = geomTreeBase.o primMST.o guibasMST.o
SRCS    = $(OBJS:%.o=%.cxx)
LIBPATHS= $(SODIRKEY)$(LIBBASE)
LIBS    = -L$(LIBBASE) $(SODIRKEY). $(LIBPATHS) -l$(THISLIB)\
	  -lABKCommon -lGeoms -lPlacement -lCombi -lStats
SLIBS   = -L$(LIBBASE) -l$(THISLIB)\
	  -lABKCommon -lGeoms -lPlacement -lCombi -lStats

PURIFY   = purify -cache_dir=cache -windows=no -log_file=purify.log\
	   -always_use_cache_dir

# ################
# standard targets
# ################

all  : lib

lib  : fs_objs $(OBJS)
	/bin/rm -f $(ARNAME) $(SODIR)/$(SONAME)
	$(AR)  $(ARNAME) $(OBJS) $(FSOBJS)
	$(LD) $(INCLUDES) -o $(SOFULL) $(OBJS) $(FSOBJS) $(SOLABEL)

fs_objs:
	cd $(CURDIR)/FastSteiner; $(MAKE) PWD=$(CURDIR)/FastSteiner CC="$(CC)" INCLUDES="$(INCLUDES)" umpack

.cxx.o:
	$(CC) $(INCLUDES) -c $*.cxx

.h.h.gch:
	$(CC) $(INCLUDES) $(ABKSYMBOLS) $*.h

.cxx.exe:
	/bin/rm -f $*.exe
	$(CC) $(INCLUDES) -c $*.cxx
	$(LDFINAL) -o $*.exe $*.o $(LIBS)

.cxx.exeS:
	/bin/rm -f $*.exeS
	$(CC) $(INCLUDES) -c $*.cxx
	$(LDFINAL) -o $*.exe $*.o -static $(SLIBS)


test: GeomTreesTest0.exe GeomTreesTest2.exe GeomTreesTest3.exe


%.exe: %.o lib
	/bin/rm -f $*.exe
	$(LDFINAL) -o $*.exe $*.o $(LIBS)

%.exeS: %.o lib
	/bin/rm -f $*.exeS
	$(LDFINAL) -o $*.exeS $*.o -static $(SLIBS)


depend:
	/bin/rm -f depend.mk

depend.mk:
	$(CC) $(DEPENDOPT) $(INCLUDES) $(SRCS) *.cxx >depend.mk

# include depend.mk

clean: 
	/bin/rm -rf Templates.DB *.exe* TimeTest lib* *.o $(OBJS) \
        core cache ir.out $(FSOBJS) SunWS_cache *notime new.out\
	*.g.gch
