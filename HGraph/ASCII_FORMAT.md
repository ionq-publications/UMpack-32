# HGraph ASCII File Formats

This note describes the ASCII hypergraph formats accepted by the HGraph parser.
It is based on `hgRead.cxx`, `hgWrite.cxx`, and examples in `HGraph/TESTS`.

For a new converter, prefer the Bookshelf-style `.aux + .nodes + .nets`
format, with optional `.wts` for node and net weights. The older `.netD + .are`
format is still accepted, but it has stronger naming assumptions.

## File Sets

An input is normally named by an `.aux` file.

```text
HGraph : design.nodes design.nets design.wts
```

or, for the legacy format:

```text
HGraph : design.netD design.are
```

Recognized `.aux` keywords are case-insensitive and include `HGraph`,
`HGraphWPins`, `PartProb`, `FPPROBLEM`, and `RowBasedPlacement`. A `CD : path`
line may appear before the design line to set a directory prefix for relative
filenames.

Do not mix `.netD/.are` files with `.nodes/.nets/.wts` in one `.aux` file.

## Comments and Whitespace

Lines whose first nonblank character is `#` are skipped where the parser calls
the common `eathash` helper. The examples use this for headers and pin comments.

Whitespace separates tokens. The parser accepts both Unix and DOS line endings.

## Recommended Format: `.nodes`

The `.nodes` file declares all node names and which nodes are terminals.

```text
UCLA nodes 1.0
# optional comments
NumNodes : 3
NumTerminals : 1
p1 terminal
a0
a1
```

Required header:

```text
UCLA nodes 1.0
NumNodes : <num_nodes>
NumTerminals : <num_terminals>
```

Each remaining node record starts with a node name. If any later token on that
line equals `terminal`, case-insensitively, the node is treated as a terminal.
Other tokens on the line are ignored by the HGraph parser.

Important parser behavior:

- The file must contain exactly `NumNodes` node records.
- The number of records marked `terminal` must equal `NumTerminals`.
- The parser stores terminals first internally, followed by nonterminals,
  regardless of their textual order in the file.
- Node names should be unique. Later lookups in `.nets` and `.wts` use the name
  map built from this file.

The examples often use terminal names `p1`, `p2`, ... and nonterminal names
`a0`, `a1`, ..., but the `.nodes/.nets/.wts` parser does not require that naming
scheme.

## Recommended Format: `.nets`

The `.nets` file declares hyperedges and their incident nodes.

```text
UCLA nets 1.0
# optional comments
NumNets : 2
NumPins : 5
NetDegree : 2 net0
a0 B
a1 B
NetDegree : 3 net1
p1 B
a0 B
a1 B
```

Required header:

```text
UCLA nets 1.0
NumPins : <num_pins>
```

`NumNets : <num_nets>` may appear before `NumPins`; the current parser reads and
skips the `NumNets` line but does not validate it. `NumPins` controls the main
read loop and must equal the number of pin records consumed.

Each net begins with:

```text
NetDegree : <degree> [net_name]
```

followed by `<degree>` pin records:

```text
<node_name> <direction>
```

The optional `net_name` is stored and can later be used by `.wts` for net
weights. If no net name is present, the net is still accepted, but named net
weights cannot refer to it unless a generated name is used by writer output.

Directions are case-insensitive after reading. Accepted tokens are:

- `B`: bidirectional or directionless pin.
- `I`: input/sink pin.
- `O`: output/source pin.

In the normal build without `SIGNAL_DIRECTIONS`, all three accepted directions
are stored as generic source/sink pins. For converter output, `B` is the safest
choice unless directionality is intentionally needed by a build that defines
`SIGNAL_DIRECTIONS`.

Important parser behavior:

- Nets of degree less than 2 are rejected.
- Pin node names must exist in the `.nodes` file.
- Extra tokens after the direction on a pin line are skipped. Existing examples
  use this for pin offsets such as `: 30 30`, but HGraph does not store those
  values here.
- Duplicate node names within one net are ignored after the first occurrence.
  They still increment the consumed pin count, so generated files should avoid
  duplicates.
- The parser stops after consuming `NumPins` pin records. If the count is too
  large, parsing reaches EOF and fails; if it is too small, trailing nets are not
  read.

## Optional Weights: `.wts`

The `.wts` file belongs to the `.nodes/.nets` path and stores node weights and
net weights.

```text
UCLA wts 1.0
a0 10.0
a1 3.0 4.0
p1 0.0
net0 2.5
```

The first line must be:

```text
UCLA wts 1.0
```

Each later line starts with a node or net name. If the name matches a net, the
next numeric value is used as that net's weight. If it matches a node, all
numeric values on that line are stored as that node's multiweights.

Important parser behavior:

- If a `.wts` file is absent, all node weights default to one.
- If a `.wts` file is present but contains no numeric weight columns, all node
  weights default to one.
- The number of node weight dimensions is inferred from the first data line.
  Names starting with digits are therefore unsafe.
- Node and net names share the same namespace for `.wts`; if a name matches both
  a node and a net, the parser gives preference to the net.
- Net weights are stored as read; large values are not clamped by the parser.
- Missing node entries leave their allocated weights at zero when a `.wts` file
  with weight dimensions is present. Converter output should write every node.

## Legacy Format: `.netD`

The `.netD` format stores topology in a compact form and uses implicit node
names.

```text
0
13
5
7
3
p1 s O
a0 l I
a1 l I
a0 s O
a2 l I
```

The header fields are read as:

```text
0
<num_pins>
<num_nets>
<num_modules>
<pad_offset>
```

If `pad_offset` is zero, the parser uses `num_modules - 1`. The number of
terminals is computed as:

```text
num_terminals = num_modules - pad_offset - 1
```

The parser then creates implicit names:

- Terminals: `p1`, `p2`, ...
- Nonterminals: `a0`, `a1`, ...

After the header, each line begins with a node name and a marker. A marker of
`s` starts a new net; other markers such as `l` add pins to the current net.
The optional third token is a direction. Accepted directions are `B`, `I`, `O`,
and `1`; if no valid direction is found, the parser defaults to `1`.

Important parser behavior:

- A net is created only if the accumulated pin list has more than one node.
- The stored net count is compared only with a warning in some cases.
- The format assumes `pN` and `aN` naming. It is less suitable for arbitrary
  HDF5 object names unless the converter first remaps them.

## Legacy Weights: `.are` / `.areM`

The `.are` and `.areM` files are used with `.netD`. They assign one or more
weights to the implicit `pN` and `aN` names.

```text
a0 1
a1 3
a2 4
p1 0
```

Each line has:

```text
<node_name> <weight0> [weight1 ...]
```

The number of weight dimensions is inferred from the first line. The parser maps
`pN` to terminal index `N - 1` and `aN` to nonterminal index
`N + num_terminals`.

If no `.are` or `.areM` file is supplied with `.netD`, all node weights default
to one.

## Converter Recommendations

For HDF5 conversion, emit:

- `design.aux`
- `design.nodes`
- `design.nets`
- `design.wts` if any node or net weight is non-unit, or if zero terminal
  weights must be preserved

Use stable generated names when the HDF5 data does not provide names. Avoid
names beginning with digits if `.wts` will be emitted. Write all nodes in
`.wts` whenever any multiweight dimension is used.

Prefer `B` for all pin directions unless the downstream flow explicitly needs
directed pins and is built with direction support.

Keep all generated nets at degree 2 or higher. If the source HDF5 data contains
single-pin hyperedges, drop them or record them outside HGraph input; the HGraph
ASCII parser rejects them.

Make `NumPins` equal the total number of pin records written in `.nets`, before
duplicate-pin elimination. Better yet, do not emit duplicate nodes within a net.
