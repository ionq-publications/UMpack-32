# ParquetDBFromRBP

ParquetDBFromRBP converts row-based placement output into Parquet DB
form used by the Parquet-related flow.

## Key Files

- `ParquetDBFromRBP.h`
- `ParquetDBFromRBP.cxx`
- `ParquetDBFromRBPTest0.cxx`
- `TESTS/`

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

For direct conversion runs, this executable is the main entry point:

- `ParquetDBFromRBPTest0`: converts an input design into Parquet DB form,
  optionally runs floorplanning, and can save the converted design.
  It accepts either:
  - `RowBasedPlacement` aux input directly
  - `PartProb` aux input by first generating an in-memory `RBPlacement`
    layout, with optional `-AR` and `-WS` controls for that synthetic layout
