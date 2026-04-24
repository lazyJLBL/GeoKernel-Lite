# Algorithms

## Convex Hull

`convexHullAndrew(points, options)` implements Andrew monotone chain. It removes duplicate points, handles empty/single/two-point inputs, detects all-collinear inputs, and supports `keepCollinear`.

`convexHullGraham(points, options)` is exposed as a compatibility entry point and currently delegates to the same robust hull implementation.

## Rotating Calipers

`convexDiameter(hull)` returns the farthest pair on a convex hull. `minimumAreaBoundingRectangle(hull)` evaluates edge-aligned candidate rectangles and returns corners, area, width, height, and angle.

## Segment Intersection

`findSegmentIntersections(segments, options)` returns all intersecting segment pairs with `None`, `Point`, or `Overlap` classification. The trace records sorted endpoint events and candidate checks.

## Half-Plane Intersection

`halfPlaneIntersection(halfPlanes, options)` clips a configurable bounding box by each half-plane. This gives stable visualization behavior for bounded and unbounded regions. Unbounded results are reported when the final polygon touches the configured bounding box.

## Closest Pair

`closestPair(points, options)` implements divide-and-conquer closest pair with duplicate detection. Duplicate points immediately return distance `0`.

## Polygon Clipping

`sutherlandHodgmanClip(subject, clipper, options)` clips a subject polygon against a convex clipper. The trace records the intermediate polygon after every clip edge.

## Ear Clipping Triangulation

`triangulateEarClipping(polygon, options)` normalizes polygon orientation, removes duplicate and collinear vertices, rejects self-intersections, and clips ears until the polygon is triangulated.

## Experimental Delaunay

`delaunayTriangulation(points, options)` implements a Bowyer-Watson prototype. It is useful for visualization and portfolio discussion, but remains marked experimental.
