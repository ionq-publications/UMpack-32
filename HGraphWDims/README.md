# HGraphWDims

HGraphWDims extends `HGraph` with node dimensions, edge weights, and
delay information used by placement and timing-related flows.

## Key Files

- `hgWDims.h`
- `hgDelayInfo.h`
- `hgWDRead.cxx`
- `hgWDWrite.cxx`
- `hgWDManual.cxx`

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

For direct weighted-hypergraph runs, these executables are the most useful
entry points:

- `HGraphWDimsTest0`: statistics and dimension-setting demo with optional
  save/export flags.
- `HGraphWDimsTest1`: semantic checks for node dimensions, weights, and copy
  behavior.
