# GeomTrees

GeomTrees provides geometric tree and Steiner-tree helpers, including
MST-based implementations and the `FastSteiner/` subpackage.

## Key Files

- `geomTreeBase.h`
- `primMST.h`
- `guibasMST.h`
- `mst2.h`
- `FastSteiner/README`

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

For direct tree and MST runs, these executables are the most useful entry
points:

- `GeomTreesTest0`: Prim MST benchmark on a point set read from `-in`.
- `GeomTreesTest1`: geometric tree edge and MST semantic checks.
- `GeomTreesTest2`: Prim MST benchmark on a point set read from `-in`.
- `GeomTreesTest3`: point-to-MST cost benchmark for a small hard-coded
  placement.
