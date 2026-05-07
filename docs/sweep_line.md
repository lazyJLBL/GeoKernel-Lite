# Sweep-Line Segment Intersection

`findSegmentIntersections` now routes to `sweepLineSegmentIntersections` by default.
The old all-pairs implementation remains available as `bruteForceSegmentIntersections`
for tests and benchmarks.

## Behavior

The sweep implementation:

- sorts segment endpoint events by x-coordinate
- maintains an active set of segments whose x-interval intersects the current sweep
  position
- checks newly active segments against the active set
- reports each pair at most once
- classifies intersections with `PredicateMode::FilteredExact` by default

Reported intersection types remain compatible with the original JSON contract:

- `none`
- `point`
- `overlap`

## Degenerate Cases

The implementation handles:

- ordinary crossings
- endpoint touches
- vertical segments
- zero-length segments
- shared endpoints
- collinear overlaps
- multiple segments meeting at one point

Collinear overlaps are reported pairwise by this API. Use `segment_arrangement` when a
split graph with deduplicated nodes and atomic subsegments is needed.

## Complexity

The event sorting step is `O(n log n)`. Candidate checks are limited to x-overlapping
active segments, so sparse inputs avoid the full all-pairs scan. Dense cases can still
approach `O(n^2 + k)`, where `k` is the number of reported pairs.

The benchmark target prints brute-force and sweep timings for fixed generated segment
sets so the difference is visible without changing the CLI contract.
