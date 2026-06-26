# ClusteredHGraph

ClusteredHGraph builds successively smaller hypergraphs from a source
`HGraph` for multilevel partitioning and placement.

## Key Files

- `baseClustHGraph.h`
- `clustHGraph.h`
- `clustHGTreeBase.h`
- `clustHGNet.h`
- `clustHGCluster.h`
- `DOCS/overview`

## Test Executables

- `ClusteredHGraphTest0` builds a clustered hypergraph from a partitioning
  problem, reports the level-by-level cuts, and saves the clustered solution.
- `ClusteredHGraphTest1` does the same starting from a fillable hierarchy and
  also writes the top-level graph as `HGTop`.

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
