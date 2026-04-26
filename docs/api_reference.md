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

## Robustness Helpers

- `EPS`
- `sign(double)`
- `equals(double, double)`
- `equals(Point2D, Point2D)`
- `lessThan(double, double)`
- `lessOrEqual(double, double)`
- `orientation(Point2D, Point2D, Point2D)`
- `PredicateMode`
- `orient2dEps(...)`
- `orient2dFiltered(...)`
- `orient2dExact(...)`
- `incircleEps(...)`
- `incircleFiltered(...)`
- `incircleExact(...)`
- `PredicateComparisonResult`
- `compareOrient2d(...)`
- `compareIncircle(...)`

## Algorithms

- `convexHullAndrew(points, options)`
- `convexHullGraham(points, options)`
- `convexDiameter(hull, options)`
- `minimumAreaBoundingRectangle(hull, options)`
- `findSegmentIntersections(segments, options)`
- `sweepLineSegmentIntersections(segments, options, predicateMode)`
- `bruteForceSegmentIntersections(segments, options, predicateMode)`
- `halfPlaneIntersection(halfPlanes, options)`
- `closestPair(points, options)`
- `sutherlandHodgmanClip(subject, clipper, options)`
- `triangulateEarClipping(polygon, options)`
- `delaunayTriangulation(points, options)`

## CLI

```powershell
geokernel_demo --algorithm <name> --input <file.json> --output <file.json> --trace --pretty
```

Supported algorithm names:

- `convex_hull`
- `rotating_calipers`
- `segment_intersection`
- `predicate_compare`
- `half_plane_intersection`
- `polygon_clipping`
- `closest_pair`
- `triangulation`
- `delaunay`

`segment_intersection` accepts optional `predicate_mode` in the input payload:

- `eps`
- `filtered_exact`
- `exact`

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
