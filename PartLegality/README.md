# PartLegality

PartLegality provides partition legality and balance checking helpers,
including generators and checkers for balanced solutions.

## Key Files

- `partLegXFace.h`
- `1balanceGen.h`
- `1balanceChk.h`
- `bfsGen.h`
- `multBalChk.h`
- `ordering.h`

## Test Executables

- `PartLegalityTest0` generates a balanced solution and checks it with pin
  constraints.
- `PartLegalityTest1` starts from an intentionally imbalanced partitioning
  and enforces balance.
- `PartLegalityTest2` combines balance generation with legality enforcement.
- `PartLegalityTest3` repeats the legality/balance flow using the BFS-based
  generator.

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

- `TODO` contains older follow-up items kept with the package.
