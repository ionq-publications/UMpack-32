# ABKCommon

ABKCommon is the shared utility library used by the UMpack projects.

It provides common support code such as assertions, timers, command-line parsing, string helpers, and version/reporting utilities.

## Key Files

- `abkassert.cxx`
- `abkrand.h`
- `abkstring.h`
- `verbosity.h`
- `abk_hash_common.h`: hash helpers for legacy string-keyed unordered
  containers.
- `MD5/`

## Build

Optimized builds use `./makeOpt-gnu`.

```sh
./makeOpt-gnu
```

For a debug build, use `./make-gnu`.

## Test

```sh
./regression
```

For direct smoke tests, these executables are the most useful entry points:

- `ABKCommonTest0`: command-line parameter parsing and flag handling.
- `ABKCommonTest1`: timer expiration smoke test.
- `ABKCommonTest2`: random-number reproducibility and reseeding.
- `ABKCommonTest13`: MD5 and CRC32 digest checks.
- `ABKCommonTest16`: correlated normal RNG sampling and statistics.
- `ABKCommonTest22`: comprehensive semantic regression for core helpers.

The regression combines the legacy compatibility baseline with deterministic
semantic checks for parameters, verbosity, numeric helpers, hashes, random
generators, timers, stream parsing, memory-mapped input, and message ordering.
It returns nonzero on the first failure.

Run the semantic checks under AddressSanitizer and UndefinedBehaviorSanitizer with:

```sh
./make-gnu sanitizer-regression
```

## Linkage

Builds produce both static and shared libraries:

- `libABKCommon.a`
- `libABKCommon.so`
