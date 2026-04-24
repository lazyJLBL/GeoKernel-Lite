# Robustness

GeoKernel-Lite centralizes floating-point decisions in `EPS`, `sign()`, `equals()`, `lessThan()`, and `lessOrEqual()`. Algorithms use these helpers instead of scattering raw `fabs(x) < 1e-9` checks.

## Predicate Policy

- Orientation is computed with `sign(cross(a, b, c))`.
- Point equality is EPS-based for both coordinates.
- Segment containment requires collinearity and bounding-box inclusion with tolerance.
- Point-in-polygon distinguishes `Inside`, `Outside`, and `OnBoundary`.
- Segment intersection distinguishes `None`, `Point`, and `Overlap`.

## Normalization

The shared normalization utilities are:

- `removeDuplicatePoints()`
- `removeCollinearVertices()`
- `ensureCounterClockwise()`
- `normalizePolygon()`
- `sortPointsLexicographically()`

These utilities are used by convex hull, clipping, triangulation, and Delaunay preprocessing.

## Visualization Policy

Unbounded half-plane intersections are clipped by a configurable bounding box. The result reports `Unbounded` when the final polygon touches the visualization box, and includes a warning so callers know the region was clipped for display.
