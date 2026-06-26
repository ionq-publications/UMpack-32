# SmallPlacement

SmallPlacement defines the small placement problem format and the core
data structures used by the small-placement solvers.

## Key Files

- `smallPlaceProb.h`
- `smallNetlist.h`
- `smallPlRow.h`
- `baseSmallPlPr.h`
- `xInterval.h`
- `DOCS/README`
- `DOCS/SPlProbFileFormat1.1`

## Test Executable

- `SmallPlacementTest0` loads a small-placement benchmark from `-f` and
  writes it back out as `savedProb.gen` so the parser and serializer can be
  checked.

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
