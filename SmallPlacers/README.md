# SmallPlacers

SmallPlacers provides the row-based and dynamic-programming placers that
operate on the small placement problem format.

## Key Files

- `baseSmallPl.h`
- `sRowSmPlacer.h`
- `sRowDPPlacer.h`
- `twoRowDPPlacer.h`
- `spNetStacks.h`
- `stripNetStacks.h`

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

For placement-oriented runs, these executables are the most useful entry
points:

- `SmallPlacersTest0`: runs the branching small placement solver repeatedly
  on an `.aux` design and can optionally save the best solution or plot it.
- `SmallPlacersTest1`: computes a bound with one branch-and-bound pass and
  then repeats the small placement search for timing/quality comparisons.
- `mbtest`: sanity-checks the `mbitset` helper used by the small placer data
  structures.
