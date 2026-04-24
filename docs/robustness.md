# Robustness Design

Computational geometry code usually fails at the boundary: nearly collinear points, overlapping segments, duplicated vertices, reversed polygon orientation, and empty or degenerate intersections. GeoKernel-Lite treats those cases as part of the public contract rather than incidental behavior.

## Floating-Point Policy

The project centralizes floating-point decisions in a small set of helpers:

- `EPS = 1e-9`
- `sign(double)`
- `equals(double, double)`
- `equals(Point2D, Point2D)`
- `lessThan(double, double)`
- `lessOrEqual(double, double)`

Algorithms should use `sign(cross(...))` for geometric orientation and `equals(...)` for semantic equality. Raw checks such as `fabs(x) < 1e-9` should not be scattered through algorithm code.

## Equality vs Ordering

`Point2D::operator==` uses EPS-based equality because geometric inputs often contain tiny numerical noise.

`Point2D::operator<` deliberately uses strict raw lexicographic ordering. This is required by `std::sort`, which needs a strict weak ordering. EPS-based less-than comparison can become non-transitive for chained values such as `0`, `0.75e-9`, and `1.5e-9`, which would make sorting behavior undefined.

The intended pattern is:

- Use strict ordering only to make containers and sorting deterministic.
- Use `equals(Point2D, Point2D)` to decide whether two points are geometrically the same.
- Run `removeDuplicatePoints()` after sorting when an algorithm needs EPS-aware duplicate removal.

## Orientation and Cross Products

Orientation is classified with:

```cpp
sign(cross(a, b, c))
```

The return value is:

- `+1`: counter-clockwise turn
- `0`: collinear under EPS
- `-1`: clockwise turn

This policy is shared by segment intersection, convex hull, polygon normalization, clipping, and triangulation. A shared orientation function keeps the behavior consistent across modules.

## Segment Intersection

Segment intersection returns a structured result:

- `IntersectionType::None`
- `IntersectionType::Point`
- `IntersectionType::Overlap`

This avoids losing important geometric information. Endpoint touches and true crossing intersections both produce `Point`, while collinear partial or full overlaps produce `Overlap` with an overlap segment.

Covered cases include:

- ordinary crossing
- endpoint touching
- zero-length segments
- parallel disjoint segments
- collinear disjoint segments
- partial overlap
- full overlap
- near-collinear inputs under EPS

## Point in Polygon

Point-in-polygon returns:

- `Outside`
- `Inside`
- `OnBoundary`

The boundary state is intentionally explicit. CAD, GIS, and clipping code often need to distinguish a point exactly on an edge or vertex from a point strictly inside a polygon.

The implementation first tests segment containment for every edge, then applies ray casting for strict inside/outside classification.

## Polygon Normalization

Polygon algorithms use shared preprocessing utilities:

- `removeConsecutiveDuplicatePoints()`
- `removeCollinearVertices()`
- `ensureCounterClockwise()`
- `normalizePolygon()`

This is especially important for triangulation. Ear clipping assumes a simple counter-clockwise polygon without redundant adjacent vertices. The triangulation module normalizes orientation, removes repeated points, filters collinear vertices, and rejects self-intersecting polygons before ear detection.

## Convex Hull Degeneracy

`convexHullAndrew()` handles:

- empty input
- one point
- two points
- duplicated points
- all-collinear point sets
- optional retention of collinear boundary points

For all-collinear input, `keepCollinear = false` returns the two endpoints and `keepCollinear = true` returns the sorted boundary points.

## Clipping and Degenerate Polygons

Sutherland-Hodgman clipping returns a status:

- `Empty`
- `Polygon`
- `Degenerate`

The `Degenerate` status is used when the output has fewer than three vertices or effectively zero area. This makes it clear when clipping collapsed a polygon into a line or point.

## Half-Plane Intersection

Half-plane intersection clips an initial bounding box by each half-plane. This is a deliberate visualization policy: unbounded regions need a finite display area.

The result status is:

- `Empty`
- `BoundedPolygon`
- `Unbounded`
- `Degenerate`

If the final polygon touches the configured bounding box, the result is reported as `Unbounded` and includes the warning `intersection_clipped_by_bounding_box`.

## Closest Pair

The closest-pair implementation detects duplicated points before divide-and-conquer recursion. If duplicates exist, the result is immediately valid with distance `0`.

The divide-and-conquer path keeps a strip around the split line and compares candidate points by y-coordinate, while trace output records divide and merge-strip phases for visualization.

## Triangulation Validation

Ear clipping reports:

- generated triangles
- original polygon area
- sum of triangle areas
- absolute area error
- validity flag

The key invariant is:

```text
sum(triangle areas) ~= polygon area
```

This gives an engineering-level correctness check beyond simply returning `n - 2` triangles.

## Known Limits

- The predicates are EPS-based, not exact arithmetic predicates.
- The current segment intersection search is an all-pairs implementation with sweep-line style trace output, not a full Bentley-Ottmann implementation.
- Half-plane intersection is implemented through iterative polygon clipping for stable visualization behavior.
- Delaunay triangulation is experimental and should be treated as a prototype for non-degenerate point sets.
