# Algorithms

## Convex Hull

`convexHullAndrew(points, options)` implements Andrew monotone chain. It removes
duplicate points, handles empty/single/two-point inputs, detects all-collinear inputs,
and supports `keepCollinear`.

`convexHullGraham(points, options)` is exposed as a compatibility entry point and
currently delegates to the same robust hull implementation.

## Rotating Calipers

`convexDiameter(hull)` returns the farthest pair on a convex hull.
`minimumAreaBoundingRectangle(hull)` evaluates edge-aligned candidate rectangles and
returns corners, area, width, height, and angle.

## Segment Intersection

`findSegmentIntersections(segments, options, predicateMode)` defaults to the sweep-line
implementation and returns all intersecting segment pairs with `None`, `Point`, or
`Overlap` classification. The trace records sorted endpoint events, active-set candidate
checks, and per-x sweep progress.

`bruteForceSegmentIntersections(segments, options, predicateMode)` is retained as a
correctness oracle for tests and benchmarks.

The CLI `segment_intersection` path defaults to `filtered_exact` predicates. Use
`predicate_mode: "eps"` in the input payload when reproducing legacy EPS behavior.

## Predicate Comparison

`compareOrient2d(...)` and `compareIncircle(...)` report EPS, filtered exact, and exact
sign classifications for the same input. The CLI exposes this as `predicate_compare`.
It is used by the robustness gallery to show when EPS collapses a nonzero orientation or
incircle determinant to zero.

## Half-Plane Intersection

`halfPlaneIntersection(halfPlanes, options)` clips a configurable bounding box by each
half-plane. This gives stable visualization behavior for bounded and unbounded regions.
Unbounded results are reported when the final polygon touches the configured bounding
box.

## Closest Pair

`closestPair(points, options)` implements divide-and-conquer closest pair with duplicate
detection. Duplicate points immediately return distance `0`.

## Polygon Clipping

`sutherlandHodgmanClip(subject, clipper, options)` clips a subject polygon against a
convex clipper. The trace records the intermediate polygon after every clip edge.

## Ear Clipping Triangulation

`triangulateEarClipping(polygon, options)` normalizes polygon orientation, removes
duplicate and collinear vertices, rejects self-intersections, and clips ears until the
polygon is triangulated.

## Experimental Delaunay

`delaunayTriangulation(points, options)` implements a Bowyer-Watson prototype. It is
useful for visualization and portfolio discussion, but remains marked experimental.
