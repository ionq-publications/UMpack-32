# FilledHier

FilledHier builds hierarchy-aware hypergraph structures used by Capo and
other multilevel flows.

## Key Files

- `fillHier.h`
- `hgForHierarchy.h`
- `fillHierParams.cxx`
- `hgForHierarchy.cxx`

## Test Executable

- `FilledHierTest0` loads a hierarchy and leaf hypergraph, fills the
  hierarchy, and writes the resulting `newHCL.hcl`.

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
