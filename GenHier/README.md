# GenHier

GenHier provides generic hierarchy-generation utilities and the small
support code used by related hierarchical packages.

## Key Files

- `genHier.h`
- `genHier.cxx`
- `prefix.cxx`
- `TESTS/`

## Test Executables

- `GenHierTest0` reads an HCL file from `-f` and writes a regenerated
  `newHCL.hcl`.
- `GenHierTest1` builds a hierarchy from a fixed list of names and delimiters
  and saves `test1.hcl`.
- `GenHierTest2` interactively reads names and delimiters, then saves
  `test2.hcl`.
- `GenHierTest3` exercises the common-prefix helper used by hierarchy parsing.
- `GenHierTest4` builds hierarchies with and without an explicit delimiter set
  and saves both outputs.

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
