# HGraph

HGraph provides the core hypergraph data structures, file I/O, and
consistency helpers used by the rest of UMpack.

## Key Files

- `hgBase.h`
- `hgFixed.h`
- `subHGraph.h`
- `cluHGraph.h`
- `hgRead.cxx`
- `hgWrite.cxx`
- `ASCII_FORMAT.md`

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

For direct hypergraph runs, these executables are the most useful entry
points:

- `HGraphTest0`: prints edge, weight, and node-degree statistics for a
  hypergraph loaded from `-f`.
- `HGraphTest1`: runs the same statistics on a clustered version of the
  hypergraph.
- `HGraphTest2`: semantic checks for construction, adjacency, weights, and
  copy behavior.
- `HGraphTest3`: validates the hypergraph API and consistency helpers.
