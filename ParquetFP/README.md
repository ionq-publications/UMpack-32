# ParquetFP

ParquetFP is a floorplanning tool for benchmark circuits in the `TESTS/` directory.

## Key Files

- `ParquetFP.h`
- `ParquetFP.cxx`
- `ParquetFPTest0.cxx`
- `TESTS/`

## Build

Optimized builds use `./makeOpt-gnu`.

Use the project wrapper for the supported build:

```sh
./makeOpt-gnu
```

This builds the main executable and the test binaries in `ParquetFP/`.
For a debug build, use `./make-gnu`.

## Run

```sh
./Parquet.exe -f TESTS/ami33
```

Use `-s <seed>` to make a run repeatable:

```sh
./Parquet.exe -f TESTS/ami33 -s 0
```

Print the full command-line help with:

```sh
./Parquet.exe -h
```

## Plot output

Add `-plot` to write a Gnuplot script named `out.gpl`:

```sh
./Parquet.exe -f TESTS/ami33 -plot
gnuplot out.gpl
```

Plot controls:

- `-plotNoNets`
- `-plotNoSlacks`
- `-plotNoNames`

## Regression

Run the provided regression script from `ParquetFP/`:

```sh
./regression
```

The regression combines deterministic semantic checks with CLI smoke tests for
both floorplan representations, soft blocks, fixed outlines, initial placement,
quadratic initialization, multilevel solving, serialization, and malformed input.
It returns nonzero on the first failure.

Run the focused semantic checks under AddressSanitizer and UndefinedBehaviorSanitizer with:

```sh
./make-gnu sanitizer-regression
```

For direct floorplanning runs, these executables are the most useful entry
points:

- `ParquetFPTest0`: semantic smoke test for floorplan representations,
  BTree/DB consistency, and round-trip database I/O.
- `ParquetFPTest1`: plot-generation regression that checks the structural
  contents of the Gnuplot output under different combinations of nets, names,
  and slack labels.
- `FPEvalTest1`: floorplan evaluation driver that can plot, save, and convert
  an existing placement into a sequence-pair.
- `FPEvalTest2`: repeated evaluation and compaction driver with soft-block
  handling and save/plot support.
- `FPEvalTest3`: analytical floorplanning driver based on the quadratic
  placement flow.
- `FPEvalTest4`: multilevel hierarchical floorplanning driver with optional
  final compaction.

## Inputs

The base name passed to `-f` should match a set of benchmark files:

- `.blocks`
- `.nets`
- optional `.pl`
- optional `.wts`

## Notes

- `expected.out` is the baseline used by the regression script.
