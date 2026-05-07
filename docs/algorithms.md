# Algorithms

## Convex Hull

`convexHullAndrew(points, options)` implements Andrew monotone chain. It removes
duplicate points, handles empty/single/two-point inputs, detects all-collinear inputs,
and supports `keepCollinear`. Orientation and duplicate policies use
`options.predicates`.

`convexHullGraham(points, options)` is exposed as a compatibility entry point and
currently delegates to the same robust hull implementation.

## Rotating Calipers and Hull Rectangle

`convexDiameter(hull)` returns the farthest pair on a convex hull using a true
rotating-calipers scan. The input is expected to be a convex hull in cyclic order. Empty,
single-point, two-point, and all-collinear hulls are handled explicitly. For nondegenerate
convex hulls the scan is `O(h)` after the hull has already been constructed.

`bruteForceConvexDiameter(hull)` remains available as an `O(h^2)` oracle for tests.

`minimumAreaBoundingRectangle(hull)` evaluates every hull edge direction and projects all
hull vertices onto that local frame. This is an honest edge-direction scan with `O(h^2)`
work, not the fully optimized multi-caliper rectangle algorithm. It returns corners,
area, width, height, and angle.

## Segment Intersection

`findSegmentIntersections(segments, options, predicateMode)` defaults to the sweep-line
implementation and returns all intersecting segment pairs with `None`, `Point`, or
`Overlap` classification. The current implementation uses an endpoint event queue, an
ordered active sequence at the current sweep x-coordinate, predecessor/successor checks
on insertion and deletion, and special checks for vertical and same-x event groups. The
trace records sorted endpoint events, candidate checks, per-x sweep progress, and whether
oracle completion was required.

Because this public API promises all intersecting pairs, the implementation still runs
the brute-force oracle as a completion pass when the endpoint-neighbor sweep misses pairs
that would require full Bentley-Ottmann intersection events. This keeps the JSON contract
correct while making the current sweep-line limitation explicit.

The preferred C++ path is `options.predicates`; the `predicateMode` overload is retained
for compatibility.

`bruteForceSegmentIntersections(segments, options, predicateMode)` is retained as a
correctness oracle for tests and benchmarks.

The CLI `segment_intersection` path defaults to `filtered_exact` predicates. Use
`predicate_mode: "eps"` in the input payload when reproducing legacy EPS behavior.

## Segment Arrangement

`buildSegmentArrangement(segments, options)` builds an arrangement-ready graph from
segments by collecting crossings and overlap boundaries, splitting each input segment
into atomic subsegments, deduplicating nodes with `PredicateContext`, and validating
that atomic edges do not contain unsplit interior intersections. The implementation is
currently correctness-first and uses the brute-force intersection oracle.

## Predicate Comparison

`compareOrient2d(...)` and `compareIncircle(...)` report EPS, filtered exact, and exact
sign classifications for the same input. The CLI exposes this as `predicate_compare`.
It is used by the robustness gallery to show when EPS collapses a nonzero orientation or
incircle determinant to zero.

## Half-Plane Intersection

`halfPlaneIntersection(halfPlanes, options)` clips a configurable bounding box by each
half-plane. This gives stable visualization behavior for bounded and unbounded regions.
Unbounded results are reported when the final polygon touches the configured bounding
box. Half-plane side tests use `options.predicates`.

## Closest Pair

`closestPair(points, options)` implements divide-and-conquer closest pair with duplicate
detection through `options.predicates`. Duplicate points immediately return distance
`0`; distances are still double values.

## Polygon Clipping

`sutherlandHodgmanClip(subject, clipper, options)` clips a subject polygon against a
convex clipper. The trace records the intermediate polygon after every clip edge.
Boundary and clip-side tests use `options.predicates`.

## Polygon Boolean Infrastructure

`polygonBoolean(subject, clip, operation, options)` currently provides the data model,
normalization, validation, point classification, and CLI skeleton for future general
polygon boolean work. It supports `Ring2D`, `PolygonWithHoles2D`, and `MultiPolygon2D`
inputs, but the overlay operation itself is not implemented yet. The CLI returns
`polygon_boolean_operation_not_implemented` after valid input is normalized and
validated.

## Ear Clipping Triangulation

`triangulateEarClipping(polygon, options)` normalizes polygon orientation, removes
duplicate and collinear vertices, rejects self-intersections, and clips ears until the
polygon is triangulated. Ear orientation and point-in-triangle tests use
`options.predicates`.

## Experimental Delaunay

`delaunayTriangulation(points, options)` implements a Bowyer-Watson prototype. It is
useful for visualization and portfolio discussion, but remains marked experimental. The
cavity test uses `PredicateContext::incircle`. The result includes edges, a validity
flag, and a `DelaunayValidationReport` covering duplicate triangles, CCW orientation,
edge consistency, empty-circle checks, and convex-hull area coverage.
