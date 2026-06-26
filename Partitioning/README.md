# Partitioning

Partitioning provides the partitioning problem definitions, partitioning
models, and evaluator interfaces used by the rest of UMpack.

## Key Files

- `part.h`
- `partitioning.h`
- `PartitioningTest0.cxx`
- `TESTS/`
- `DOCS/`

## Test Executables

- `PartitioningTest0` saves the input problem back out in netD/are or
  nodes/nets format.
- `PartitioningTest1` prints basic size, weight, and target-capacity
  information.
- `PartitioningTest2` checks that node degrees match the incident-edge count.
- `PartitioningTest3` exercises terminal propagation for a few sample
  terminals.
- `PartitioningTest4` reports size and degree statistics for a loaded problem.
- `PartitioningTest5` prints quantized bounding-box information for a point
  set.
- `PartitioningTest6` reports node and net statistics for the loaded graph.
- `PartitioningTest7` expands the node/net statistics with histograms and
  external-net counts.
- `PartitioningTest8` builds a sub-partitioning problem from a placement and
  prints its statistics.
- `PartitioningTest9` reorders a placement so terminals come first and saves
  the result.
- `PartitioningTest10` slices a placement into several subproblems and saves
  each derived benchmark.
- `PartitioningTest11` builds one more subproblem variant and writes the
  resulting benchmark.
- `PartitioningTest12` prints the largest-cell statistics for a loaded
  partitioning problem.
- `PartitioningTest13` checks the partitioning buffer, double buffer, and
  solution metadata classes.

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

## Synopsis

Foundation classes `PartitioningProblem` (with complete I/O), `Partitioning`,
`BasePartitioner`, `BasePartEvaluator`. Working samples include
`Greedy2WayPartitioner` and `NetCut2Way` evaluator.

Samples of `PartitioningProblems` are in `Partitioning/TESTS`.
