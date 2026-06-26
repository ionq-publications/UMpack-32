# Ctainers

Ctainers provides low-level utility containers used across UMpack, such
as matrices, bit boards, priority containers, trees, and simple helper
types.

## Key Files

- `bitBoard.h`
- `dmatrix.h`
- `umatrix.h`
- `rbtree.h`
- `discretePrioritizer.h`
- `listO1size.h`
- `upair.h`

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

The regression retains the legacy output baseline and adds semantic checks for
bit boards, discrete priorities, stacks, matrices, constant-size lists, pairs,
and red-black trees. It returns nonzero on the first failure.

For direct container runs, these executables are the most useful entry points:

- `CtainersTest0`: bit-board smoke test.
- `CtainersTest1`: discrete-prioritizer exercise with bucket traversal.
- `CtainersTest2`: `RiskyStack` copy, assignment, and reserve test.
- `CtainersTest3`: comprehensive semantic regression for the container
  utilities.

Run the semantic checks under AddressSanitizer and UndefinedBehaviorSanitizer with:

```sh
./make-gnu sanitizer-regression
```
