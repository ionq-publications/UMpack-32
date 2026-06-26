# AnalytPl

AnalytPl provides analytical placement helpers used by the placement flow.
The main interface is `analytPl.h`, with the implementation in
`analytPl.cxx`.

## Key Files

- `analytPl.h`
- `analytPl.cxx`
- `AnalytPlTest0.cxx`
- `TESTS/`

## Test Executable

- `AnalytPlTest0` runs the analytical placement driver on an input `-f`
  placement, updates the movable cells, and prints the resulting placement
  statistics and optional plots.

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
