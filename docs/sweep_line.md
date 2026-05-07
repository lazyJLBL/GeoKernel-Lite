# Sweep-Line Segment Intersection

`findSegmentIntersections` now routes to `sweepLineSegmentIntersections` by default.
The old all-pairs implementation remains available as `bruteForceSegmentIntersections`
for tests and benchmarks.

## Behavior

The sweep implementation:

- sorts segment endpoint events by x-coordinate
- maintains an ordered active sequence by each segment's y-coordinate at the current
  sweep x-coordinate
- checks predecessor/successor pairs when a segment is inserted
- checks the former neighbors when a segment is removed
- performs same-x event group checks for endpoint touches and zero-length segments
- performs vertical segment checks against active segments whose sweep-y lies in the
  vertical segment's y-range
- reports each pair at most once
- classifies intersections with `PredicateMode::FilteredExact` by default

This is not yet a full Bentley-Ottmann implementation with dynamically inserted
intersection events. The public API promises all intersecting pairs, so the current
implementation runs the brute-force oracle as a completion pass and adds any pair missed
by the endpoint-neighbor sweep. When that happens, the result includes the warning
`sweep_line_oracle_completion_added_pairs`.

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

The event sorting step is `O(n log n)`. The ordered active pass checks local neighbors
and degenerate same-x/vertical groups, so sparse nondegenerate inputs avoid many
candidate checks. Because the all-pairs API is preserved through oracle completion,
worst-case time remains `O(n^2 + k)`, where `k` is the number of reported pairs.

This complexity is an honest implementation boundary: replacing oracle completion with
full event-driven Bentley-Ottmann intersection scheduling is planned separately from the
current contract-preserving sweep.

The benchmark target prints brute-force and sweep timings for fixed generated segment
sets so the difference is visible without changing the CLI contract.
