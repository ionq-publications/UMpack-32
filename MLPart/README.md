# MLPart

MLPart is the multilevel k-way partitioner used in the UMpack flow.

## Key Files

- `mlPart.h`
- `MLPartTest0.cxx`
- `TESTS/`
- `DOCS/`

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

The package includes several small drivers:

- `MLPartTest0`: stand-alone MLPart run. Supports `-num` for multiple starts,
  `-weightOption` for clustering weight selection, `-refine` to seed from an
  imported solution, and `-hcl` for an explicit hierarchy. It reports the final
  cut, part areas, nonzero-cost nets, and saves `best.sol`.
- `MLPartTest1`: repeated-run driver for best-of-n statistics. It supports
  `-runs`, `-num`, and `-refine`, and reports cost/runtime summaries.
- `MLPartTest2`: Bookshelf-style executable driver with banner output,
  repeated best-of-n runs, optional `-skipVCycle`, and optional `-saveBest`.
- `MLPartTest6`: two-stage sensitivity run. It first runs MLPart with relaxed
  fixed constraints, restores the original constraints into the best solution,
  reruns MLPart, and reports cost/runtime summaries.
- `MLPartTest7`: weighted-cut driver. It perturbs edge weights around 100,
  uses the weighted two-way net-cut evaluator, and reports weighted and
  unweighted cuts across repeated runs.
- `MLPartTest8`: seeded large-edge-weight sensitivity check. It collects five
  MLPart solutions with all edge weights set to 1, selects one cut edge per
  solution, then reruns seeded refinement with that selected edge made heavier
  in three weighting modes.

The regression script compares stable filtered output from `MLPartTest0`,
`MLPartTest1`, and `MLPartTest8`. `MLPartTest8` is filtered down to its summary
tables because MLPart still emits timing, seed, and clustering details around
the deterministic experiment.
