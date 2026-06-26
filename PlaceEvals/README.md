# PlaceEvals

PlaceEvals provides placement evaluation metrics and supporting
infrastructure for bounding-box, edge, and clustering-based costs.

## Key Files

- `placeEvalXFace.h`
- `plEvalRegistry.cxx`
- `hbboxPlEval.h`
- `hbboxWRSMTPlEval.h`
- `edgePlEval.h`
- `clustLayout.h`

## Test Executables

- `PlaceEvalsTest0` computes a chosen placement cost for a placement and
  layout loaded from `-net`, `-are`, `-blk`, and `-pl`.
- `PlaceEvalsTest1` evaluates the same placement cost but only exercises the
  per-net cost reporting path.

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
