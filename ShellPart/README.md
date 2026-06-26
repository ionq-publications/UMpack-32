# ShellPart

ShellPart provides shell-partitioning helpers built around iterative
tolerance partitioning and FM-based components.

## Key Files

- `iterTolPart.h`
- `iterTolPart.cxx`
- `bbFMPart.h`
- `bbFMPart.cxx`

## Test Executables

- `ShellPartTest1` runs the bitboard FM partitioner on a loaded problem and
  reports the resulting cut statistics.
- `ShellPartTest2` runs the iterative tolerance partitioner, prints runtime
  and expected-minimum summaries, and compares the output cuts.

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
