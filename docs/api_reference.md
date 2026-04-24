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

## Algorithms

- `convexHullAndrew(points, options)`
- `convexHullGraham(points, options)`
- `convexDiameter(hull, options)`
- `minimumAreaBoundingRectangle(hull, options)`
- `findSegmentIntersections(segments, options)`
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
- `half_plane_intersection`
- `polygon_clipping`
- `closest_pair`
- `triangulation`
- `delaunay`
