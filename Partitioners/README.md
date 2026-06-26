# Partitioners

Partitioners contains the partitioning framework, move registries, and
multi-start partitioner implementations used by the rest of UMpack.

## Key Files

- `partitioners.h`
- `partRegistry.h`
- `moveRegistry.h`
- `multiStartPart.h`
- `aGreedPart.h`
- `aGreedMoveMan.h`

## Test Executables

- `PartitionersTest0` instantiates the greedy partitioner on a benchmark and
  prints the parameters it uses.
- `PartitionersTest1` exercises command-line parsing for the partitioner
  parameter block.
- `PartitionersTest2` runs the multi-start greedy HER flow and saves the best
  solution.

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
