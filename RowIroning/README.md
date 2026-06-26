# RowIroning

RowIroning is a local-search placement refinement pass that improves
half-perimeter wirelength by sliding small windows of cells within (and
optionally across pairs of) placement rows.  For each window it formulates
a small placement sub-problem, solves it with `SmallPlacers` (branch-and-bound
or dynamic programming), and commits solutions that reduce WL.

## Key Files

- `rowIroning.h` / `rowIroning.cxx` — main row-ironing passes (1D and 2D)
- `riParams.h` / `riParams.cxx` — `RowIroningParameters`: window size, number
  of passes, overlap, algorithm selection, verbosity
- `flutePlacer.cxx` — optional FLUTE-based Steiner-tree placer (compiled only
  with `-DUSEFLUTE`)

## Build

```sh
./makeOpt-gnu        # optimized build
./make-gnu           # debug build
```

## Test Executables

- `RowIroningTest1`: loads a Bookshelf row-based placement from
  `-f design.aux`, optionally removes overlaps (`-skipLegal` to skip),
  runs `ironRows()` with configurable parameters, and reports initial and
  final pin-to-pin HPWL.  Key options:
  - `-smPlAlgo Branch | DynamicP` — small-placer algorithm
  - `-ironWindow N` — sliding window size (default 12)
  - `-ironPasses N` — number of passes (default 1)
  - `-ironTwoDim` — enable two-row (2D) ironing
  - `-noRowIroning` — skip ironing (useful to measure legalization alone)
  - `-save` — write final placement to `out.pl`
  - `-plot*` — gnuplot visualization options

> `RowIroningTest0` and `RowIroningTest2` require the DB and RBPlFromDB
> packages (not present in this repo) and are not built by the default
> `test` target.

## Test

```sh
./regression
```

The regression runs `RowIroningTest1` with branch-and-bound and dynamic
programming solvers on two Bookshelf benchmarks (C432a and
primary1.nf.WS10) and diffs the output against `expected.out`.

## Notes

- RowIroning links against FixOr for cell-orientation handling.
- The optional FLUTE Steiner-tree wirelength estimator is gated by
  `-DUSEFLUTE` at compile time.
- `RowIroningTest2` implements an iterative multi-strategy loop (stdRowIroning,
  ri2d1pass, ri2d2passes, riBBpass) and saves intermediate placements; it
  requires the DB package to construct the initial placement.
