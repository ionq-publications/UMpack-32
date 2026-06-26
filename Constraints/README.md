# Constraints

Constraints packages up placement and partitioning constraint types and
the `Constraints` manager used to query and apply them.

## Key Files

- `constraints.h`
- `allConstr.h`
- `fxTypeConstr.h`
- `regionConstr.h`
- `otherConstr.h`
- `constraint.h`

## Test Executables

- `ConstraintsTest0` exercises the main constraint classes individually.
- `ConstraintsTest1` builds and sorts a mixed constraint collection.
- `ConstraintsTest2` adds additional fixed-group cases and compactifies them.
- `ConstraintsTest3` checks restriction and pull-back behavior for mixed
  constraints.
- `ConstraintsTest4` exercises constraint restriction and pull-back with a
  different fixed-constraint mix.
- `ConstraintsTest5` builds a fixed-type constraint collection and applies it
  to a fresh placement.
- `ConstraintsTest6` runs semantic checks on the individual constraints and a
  combined collection.

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

- `README` in this directory is the original package note and is kept for
  historical reference.
