# CongestionMaps

CongestionMaps provides congestion and density map utilities used by the
placement flow.  It includes the ISPD04 and ISPD06 map variants.

## Key Files

- `CongestionMaps.h`
- `ISPD04CongMap.h`
- `ISPD06DensityMap.h`
- `baseGeneric2DResourceMap.h`
- `palette.h`

## Test Executables

- `CongestionMapsTest0` plots congestion maps from an `RBPlacement` using the
  generic probabilistic or deterministic resource-map flow.
- `CongestionMapsTest1` builds and plots the ISPD04 congestion map variant.
- `CongestionMapsTest2` converts a DB-backed placement to the ISPD04 map
  workflow before plotting.
- `CongestionMapsTest3` compares the ISPD04 and ISPD06-style resource map
  flows and exposes the contest-oriented plotting options.

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
