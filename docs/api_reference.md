# API Reference

All public C++ symbols live in `namespace geokernel`.

## Core Types

- `Point2D`
- `Vector2D`
- `Line2D`
- `Segment2D`
- `Circle2D`
- `Polygon2D`
- `HalfPlane2D`
- `Box2D`
- `Triangle2D`
- `Ring2D`
- `PolygonWithHoles2D`
- `MultiPolygon2D`
- `SplitSegment`
- `ArrangementNode`
- `ArrangementEdge`
- `DelaunayValidationReport`

## Robustness Helpers

- `EPS`
- `sign(double)`
- `equals(double, double)`
- `equals(Point2D, Point2D)`
- `lessThan(double, double)`
- `lessOrEqual(double, double)`
- `orientation(Point2D, Point2D, Point2D)`
- `PredicateMode`
- `PredicateContext`
- `orient2dEps(...)`
- `orient2dFiltered(...)`
- `orient2dExact(...)`
- `incircleEps(...)`
- `incircleFiltered(...)`
- `incircleExact(...)`
- `PredicateComparisonResult`
- `compareOrient2d(...)`
- `compareIncircle(...)`

`PredicateContext` contains:

- `mode`
- `eps`
- `orient(a, b, c)`
- `incircle(a, b, c, d)`
- `equals(a, b)`
- `compareLexicographic(a, b)`

## Algorithms

- `convexHullAndrew(points, options)`
- `convexHullGraham(points, options)`
- `convexDiameter(hull, options)`
- `minimumAreaBoundingRectangle(hull, options)`
- `findSegmentIntersections(segments, options)`
- `sweepLineSegmentIntersections(segments, options, predicateMode)`
- `bruteForceSegmentIntersections(segments, options, predicateMode)`
- `buildSegmentArrangement(segments, options)`
- `halfPlaneIntersection(halfPlanes, options)`
- `closestPair(points, options)`
- `sutherlandHodgmanClip(subject, clipper, options)`
- `triangulateEarClipping(polygon, options)`
- `delaunayTriangulation(points, options)`
- `polygonBoolean(subject, clip, operation, options)`
- `normalizeRing(...)`
- `normalizePolygonWithHoles(...)`
- `normalizeMultiPolygon(...)`
- `validateRing(...)`
- `validatePolygonWithHoles(...)`
- `validateMultiPolygon(...)`
- `pointInRing(...)`
- `pointInPolygonWithHoles(...)`
- `pointInMultiPolygon(...)`

## CLI

```powershell
geokernel_demo --algorithm <name> --input <file.json> --output <file.json> --trace --pretty
```

Supported algorithm names:

- `convex_hull`
- `rotating_calipers`
- `segment_intersection`
- `segment_arrangement`
- `predicate_compare`
- `half_plane_intersection`
- `polygon_clipping`
- `polygon_boolean`
- `closest_pair`
- `triangulation`
- `delaunay`

`segment_intersection` accepts optional `predicate_mode` in the input payload:

- `eps`
- `filtered_exact`
- `exact`

Predicate-aware algorithm inputs may also include:

```json
{
  "predicate_mode": "filtered_exact",
  "predicate_eps": 1e-9
}
```

Predicate-aware summaries include both `predicate_mode` and `predicate_eps`.

`polygon_boolean` currently validates and normalizes `MultiPolygon2D` input. It does not
yet compute the boolean overlay result.

`predicate_compare` accepts:

```json
{
  "predicate": "orient2d",
  "points": [[0, 0], [1, 0], [0.5, 1e-12]]
}
```

or:

```json
{
  "predicate": "incircle",
  "points": [[0, 0], [1, 0], [0, 1], [1, 0.999999999999]]
}
```
