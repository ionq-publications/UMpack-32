# Geoms

Geoms provides the core geometry primitives used throughout UMpack,
including points, boxes, and intervals.

## Key Files

- `point.h`
- `bbox.h`
- `interval.h`
- `ibbox.h`
- `plGeom.h`

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

For direct geometry runs, these executables are the most useful entry points:

- `GeomsTest0`: interval-sequence complement, merge, and intersection demo.
- `GeomsTest1`: interval overlap, length, blanking, and random interval
  generation demo.
- `GeomsTest2`: point, bounding-box, and interval semantic checks.
- `GeomsTest3`: interval canonicalization and randomized interval merging.
