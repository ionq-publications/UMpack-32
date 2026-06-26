# Placement

Placement provides the core geometry and placement-point classes used by
the rest of UMpack.

## Key Files

- `placement.h`
- `mapping.h`
- `subPlacement.h`
- `plStats.h`
- `transf.h`
- `compar.h`
- `rwalk.h`

## Test Executables

- `PlacementTest0` demonstrates points, boxes, placement copies, and
  coercion into bounding boxes.
- `PlacementTest1` rotates a grid placement and writes before/after outputs.
- `PlacementTest2` computes polygon area and point-in-polygon checks with a
  convex hull.
- `PlacementTest3` exercises `SubPlacement` pull-back, push-forward, and
  reorder operations.
- `PlacementTest4` groups points into a grid placement and writes the
  extracted grid ordering.
- `PlacementTest5` draws a few sample primitives into `out.xy`.
- `PlacementTest6` checks `SubPlacement` reordering against a permutation.
- `PlacementTest7` prints random dimension ranges for cells.
- `PlacementTest8` runs semantic checks for geometry, reordering, transforms,
  and layout boxes.
- `PlacementTest11` measures how clumping toward the center affects
  placement correlation.
- `PlacementTest12` exercises the seeded random-walk helper used by the
  placement utilities.

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

## Remarks

Placement is a generic pointset class that knows nothing about placement
algorithms. A standard model for placer is a derived class (for example,
`QPlacement`) which does all the work in its constructor.

Convex hull for 100,000 points takes 5 sec to compute. Computing and
destroying 100,000 convex hulls for the same 20 points takes 31 sec. MST is
pretty fast too, but it rounds all input coordinates to integers.
