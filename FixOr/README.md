# FixOr

FixOr optimizes cell flip/mirror orientations in a row-based placement to
minimize half-perimeter wirelength.  It implements a greedy gain-based
algorithm (`GreedyOrientOpt`) that iterates over cells, evaluating the
change in net bounding-box cost for each possible orientation, and accepts
flips that reduce total wirelength.

## Key Files

- `greedyOrientOpt.h` / `greedyOrientOpt.cxx` — greedy orientation optimizer
- `orientOptXFace.h` / `orientOptXFace.cxx` — base class and parameter handling
- `heapGainCtr.h` — max-heap used internally to order candidates by gain

## Build

```sh
./makeOpt-gnu        # optimized build
./make-gnu           # debug build
```

## Test Executables

- `FixOrTest1`: loads a Bookshelf row-based placement from `-f design.aux`,
  runs `GreedyOrientOpt`, and reports initial WL, number of flipped cells,
  and final WL.  Optionally saves the updated placement with `-o out.def`.

> `FixOrTest0` requires the DB and RBPlFromDB packages (not present in this
> repo) and is not built by the default `test` target.

## Test

```sh
./regression
```

The regression runs `FixOrTest1` on two Bookshelf benchmarks from the
`RowIroning/TESTS/` directory (C432a and primary1.nf.WS10) and diffs the
output against `expected.out`.

> The regression uses `../RowIroning/TESTS/` because FixOr's own `TESTS/`
> directory contains only LEF/DEF-based inputs that require the DB package.
> If the RowIroning package is moved, update the paths in `regression`.

## Notes

- The `TESTS/` directory holds LEF/DEF inputs (`quasiHP.*`) used by the
  original DB-based `FixOrTest0`.  They are retained for reference.
- `GreedyOrientOpt` supports an optional FLUTE-based Steiner-tree cost
  model; build with `-DUSEFLUTE` and link the Flute library to enable it.
