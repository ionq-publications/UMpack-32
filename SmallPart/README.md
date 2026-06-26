# SmallPart

SmallPart provides small-instance partitioning algorithms, including
branch-and-bound, brute-force, and other exact or near-exact solvers.

## Key Files

- `smallPart.h`
- `bbPart.h`
- `enumPart.h`
- `netStacks.h`
- `netStacksInevCuts.h`
- `hgraphBreakTriangles.h`

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

For direct solver runs, these executables are the most useful entry points:

- `SmallPartTest0`: runs the exact enumerator and saves the best solution.
- `SmallPartTest1`: measures the enumerator repeatedly to estimate runtime.
- `SmallPartTest2`: runs the branch-and-bound solver and checks legality and
  cut quality.
- `SmallPartTest3`: repeats branch-and-bound many times to measure average
  runtime.
- `SmallPartTest4`: runs branch-and-bound with a configurable push limit.
- `SmallPartTest5`: exercises the same branch-and-bound path with different
  default verbosity settings.
- `SmallPartTest6`: runs branch-and-bound with the explicit `-nc` flag and
  reports legality and cut statistics.
- `SmallPartTest7`: times the branch-and-bound solver repeatedly on one input.
