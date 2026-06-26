# Combi

Combi provides small combinatorial helpers used by the partitioning and
placement flows, including the Gray-code transporter utilities.

## Key Files

- `combi.h`
- `grayTransp.h`
- `main0.cxx` through `main4.cxx`

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

The package test data lives in `TESTS/`.

For direct combinatorics runs, these executables are the most useful entry
points:

- `CombiTest0`: enumerates permutations lexicographically and times the run.
- `CombiTest1`: exercises subset operations such as complements, unions, and
  intersections.
- `CombiTest2`: builds permutations from vectors and compares their
  representations.
- `CombiTest3`: walks gray-code transpositions for permutations.
- `CombiTest4`: steps through partitionings with Gray-code increments.
- `CombiTest5`: randomizes a partitioning and prints the result.
- `CombiTest6`: runs permutation and partition-id round-trip utilities.
- `CombiTest7`: semantic checks for permutation, partition-id, and gray-code
  helpers.

See `main0.cxx` through `main3.cxx` for sample uses.
See `main4.cxx` for `grayTransp.h` usage.
