# FMPart

FMPart implements Fiduccia-Mattheyses style partitioning and the move
manager infrastructure used by the multilevel partitioning flow.

## Key Files

- `fmPart.h`
- `fmPartPlus.h`
- `fmNetCut2WayMM.h`
- `fmMoveManagerNC2w.h`
- `gainBucket.h`
- `gainList.h`

## Build

Optimized builds use `./makeOpt-gnu`.

Use the package wrapper from this directory:

```sh
./makeOpt-gnu
```

For a debug build, use `./make-gnu`.

## Test

Run the package regression script:

```sh
./regression
```

`FMPartTest0` and `FMPartTest1` are the main FM drivers. When no initial
solution is available, they can be used with multiple starts to produce several
candidate partitions. When `-refine` is used on the same imported `.sol`
solution, FMPart is deterministic, so repeated runs will return the same
refined solution.

`FMPartTest8` is a deterministic weighted-edge sensitivity sweep. It runs FM
from five fixed initial starts with all edge weights set to 1, selects one cut
edge per solution, and reruns refinement for weights 1, 5, 10, 100, 1000, and
10000. The report compares three reweighting cases: selected edge `W` with all
other edges at 1, selected edge `20*W` with all other edges at `W`, and
selected edge `W+20000` with all other edges at `W`.
