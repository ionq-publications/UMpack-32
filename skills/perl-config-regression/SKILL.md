---
name: perl-config-regression
description: Use when running `perl config` in this repository and you need the standard default-answer flow through the `regression-only version` prompt, then a controlled decision on test execution based on whether all packages built cleanly. Covers collecting the full list of failing packages, the temporary memory-saving edit to `ABKCommon/config.h`, and the required reversion before commit.
---

# Perl Config Regression

Use this skill when driving the top-level `perl config` workflow from the repo
root.

## Default-answer flow

First, perform makeOpt-gnu clean in each package to avoid binary incompatibilities.
Run `perl config` in a PTY and accept defaults by sending an empty line through
the normal setup prompts up to and including the `regression-only version`
question.

This includes:

- processor-count prompt, if shown
- `Would you like to compile/build UMpack components now ? [Y/n]`
- `Would you like compiler optimization for run time ... ? [Y/n]`
- `memory-saving options ?` with the prompt's default answer
- `Your g++ supports -march=native ; would you like to use it ? [Y/n]`, if
  shown
- `Would you like to build a regression-only version ... ? [y/N]`

Auto-accept all prompts until the testing prompt. If `config` reports compiler or
runtime problems and asks whether to proceed anyway, stop and ask the user.

## Build phase behavior

Do not stop on the first package compile failure. Let the package build phase
finish so you can collect the full list of failing packages from the complete
`perl config` run.

If some packages fail to build:

- summarize the full failing-package list
- answer no at the later test prompt
- do not build or run tests unless test execution is specifically needed to
  debug an already-known test problem

If all packages build cleanly, continue to the test prompt and answer yes.

## Test prompt behavior

After the `regression-only version` prompt, `config` eventually reaches:

- `Would you like to test UMpack components now? [Y,n]`
- or `Would you like to test UMpack components now? [y,N]`

At that point:

- answer yes if the package build phase was clean
- answer no if any package builds failed, unless running tests is specifically
  needed for debugging

## Memory-saving mode

Be careful with the `memory-saving options` prompt:

- On the optimized default path, accepting defaults enables memory-saving.
- `config` implements this by removing the `SIGNAL_DIRECTIONS` block from
  `ABKCommon/config.h`.
- That change can alter regression output, so treat it as temporary unless the
  user explicitly wants to keep working in that mode.

After using memory-saving mode:

1. Restore the normal `SIGNAL_DIRECTIONS` block in `ABKCommon/config.h`.
   But no need to make clear and rebuild everything again.
2. Rebuild affected libraries and packages before final validation.
3. Confirm `git diff -- ABKCommon/config.h` is empty before committing.
