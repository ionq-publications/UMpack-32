# RBPlace

RBPlace is the row-based placement package used for legalization,
overlap removal, row management, and related placement utilities.

## Key Files

- `RBPlacement.h`
- `RBPlacement.cxx`
- `RBRows.h`
- `overlapRem.h`
- `routingResources.h`
- `saveLEFDEF.cxx`

## Test Executables

- `RBPlaceTest0` loads a placement and optionally writes it back out in
  several serialization formats.
- `RBPlaceTest1` evaluates center-to-center and pin-to-pin half-perimeter
  wirelength.
- `RBPlaceTest2` generates a new row-based layout from an input benchmark.
- `RBPlaceTest3` builds a layout and writes a placement file using the
  requested aspect ratio and whitespace.
- `RBPlaceTest4` exercises legalization, snapping, tethering, and plotting
  options on a row-based placement.
- `RBPlaceTest5` applies the same row-based operations with a smaller option
  set and is useful for quick legality checks.
- `RBPlaceTest8` slices a placement into a mini benchmark, fixing cells and
  removing nets that fall fully outside the cut window.
- `RBPlaceTest9` reorders a placement and saves it as a new benchmark.
- `RBPlaceTest9.5` rescales cell widths using driven-length heuristics and
  saves the updated layout.
- `RBPlaceTest10` compares an old placement against the current one and
  writes displacement diagnostics.

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

## Notes

- `DOCS/` contains legacy notes and examples for the row-based flow.
