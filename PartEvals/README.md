# PartEvals

PartEvals provides partition quality evaluators, tally helpers, and
evaluation registry code used by the partitioning flow.

## Key Files

- `univPartEval.h`
- `evalRegistry.h`
- `netTallies.h`
- `strayNodes.h`
- `talliesWCosts.h`
- `netCutWConfigIds.h`

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

For direct evaluator runs, these executables are the most useful entry
points:

- `PartEvalsTest0`: net-cut evaluator smoke test.
- `PartEvalsTest1`: net-cut-with-config-ids evaluator test.
- `PartEvalsTest2`: weighted net-cut evaluator test.
- `PartEvalsTest3`: bounding-box one-dimensional evaluator test.
- `PartEvalsTest4`: stray-nodes evaluator test.
- `PartEvalsTest6`: weighted net-cut with edge weights test.
- `PartEvalsTest17`: evaluator-type parsing smoke test.
- `PartEvalsTest18`: evaluator registry and cost comparison test.
- `PartEvalsTest19`: two-way stray-nodes evaluator test.
- `PartEvalsTest20`: two-way weighted net-cut evaluator test.
