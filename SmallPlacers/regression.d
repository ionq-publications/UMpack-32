#!/bin/sh

#**************************************************************************
#**
#**    Copyright (c) 1995-1997 by ABKGroup, UCLA VLSI CAD Laboratory,
#**    UCLA Computer Science Department, Los Angeles, CA 90095-1596 USA,
#**    and by the Regents of the University of California.
#**    All rights reserved.
#**
#**    No portion of this file may be used, copied, or transmitted for
#**    any purpose, including any research or commercial purpose,
#**    without the express written consent of Professor Andrew B. Kahng,
#**    UCLA Computer Science Department, 3713 Boelter Hall, Los Angeles,
#**    CA 90095-1596 USA.   Any copy made of this file must include this
#**    notice.    ABKGroup and the Regents of the University of California
#**    retain title to all copies of this file and/or its compiled
#**    derivatives.
#**
#**************************************************************************/

DIFF=/usr/bin/diff
WC=/usr/bin/wc
PROG=./SmallPlacersTest0.exe

run_bb() {
	"$PROG" -smPlAlgo Branch "$@"
}

run_dp() {
	"$PROG" -smPlAlgo DynamicP "$@"
}

/bin/rm -f seeds.out seeds.in

echo "___ branch and bound tests ______" >new.out
echo "___ size 6 tests _____" >>new.out
run_bb -f TESTS/amd_C6.aux -num 100 >>new.out
echo "___ size 8 tests _____" >>new.out
run_bb -f TESTS/pl_N8_E14_281.aux -num 20 >>new.out
run_bb -f TESTS/pl_N8_E18_591.aux -num 20 >>new.out
run_bb -f TESTS/pl_N8_E20_695.aux -num 20 >>new.out
echo "___ size 9 tests _____" >>new.out
run_bb -f TESTS/pl_N9_E19_365.aux -num 10 >>new.out
run_bb -f TESTS/pl_N9_E25_213.aux -num 10 >>new.out
run_bb -f TESTS/pl_N9_E32_337.aux -num 10 >>new.out
echo "___ size 10 tests _____" >>new.out
run_bb -f TESTS/pl_N10_E19_297.aux -num 5 >>new.out
run_bb -f TESTS/pl_N10_E24_669.aux -num 5 >>new.out
run_bb -f TESTS/pl_N10_E25_293.aux -num 2 >>new.out
echo "___ size 11 tests _____" >>new.out
"$PROG" -f TESTS/pl_N11_E26_569.aux -num 2 -verb "10_10_10" >>new.out

echo "___ dynamic programming tests ______" >>new.out
echo "___ size 6 tests _____" >>new.out
run_dp -f TESTS/amd_C6.aux -num 100 >>new.out
echo "___ size 8 tests _____" >>new.out
run_dp -f TESTS/pl_N8_E14_281.aux -num 20 >>new.out
run_dp -f TESTS/pl_N8_E18_591.aux -num 20 >>new.out
run_dp -f TESTS/pl_N8_E20_695.aux -num 20 >>new.out
echo "___ size 9 tests _____" >>new.out
run_dp -f TESTS/pl_N9_E19_365.aux -num 10 >>new.out
run_dp -f TESTS/pl_N9_E25_213.aux -num 10 >>new.out
run_dp -f TESTS/pl_N9_E32_337.aux -num 10 >>new.out
echo "___ size 10 tests _____" >>new.out
run_dp -f TESTS/pl_N10_E19_297.aux -num 5 >>new.out
run_dp -f TESTS/pl_N10_E24_669.aux -num 5 >>new.out
run_dp -f TESTS/pl_N10_E25_293.aux -num 2 >>new.out
echo "___ size 11 tests _____" >>new.out
run_dp -f TESTS/pl_N11_E26_569.aux -num 2 >>new.out
echo "___ size 10 2D tests _____" >>new.out
run_dp -f TESTS/smPbC432a.aux -num 10 >>new.out
echo "___ size 15 2D tests _____" >>new.out
run_dp -f TESTS/smPbIntel.aux -num 5 >>new.out
echo "___ size 19 2D tests _____" >>new.out
run_dp -f TESTS/smPbPrimary1.aux -num 5 >>new.out

egrep -v "(Created|took|User|Platform|Memory|Initial solution|initial ordering|Initial Bound|New Best|Total Add Operations|Final soln has cost|placements took)" new.out >newout.notime
egrep -v "(Created|took|User|Platform|Memory|Initial solution|initial ordering|Initial Bound|New Best|Total Add Operations|Final soln has cost|placements took)" expected.out >expectedout.notime

"$DIFF" newout.notime expectedout.notime >diffs.notime

echo " "
echo Differences from precomputed results
"$WC" diffs.notime

rm -f seeds.out seeds.in expectedout.notime
