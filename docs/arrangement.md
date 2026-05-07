# Segment Arrangement

The segment arrangement builder is a correctness-first topology preparation step for
future polygon boolean and overlay work. It does not replace the existing
`segment_intersection` CLI contract; it adds a richer graph-oriented output.

## Problem

Pairwise segment intersection answers whether two input segments meet. Polygon boolean
and overlay algorithms need more: every intersection and overlap boundary must become a
node, and every input segment must be split into atomic subsegments that no longer have
unsplit interior intersections.

## Algorithm

`buildSegmentArrangement(segments, options)` currently uses the brute-force
intersection oracle for correctness:

1. add every segment endpoint as a split point
2. find all pairwise point and overlap intersections
3. add crossing points and overlap boundaries to the affected segments
4. sort split points along each segment
5. build atomic subsegments between consecutive split points
6. deduplicate graph nodes with `PredicateContext`
7. create directed arrangement edges that preserve source segment direction
8. validate that atomic edges have no unsplit interior intersections

This is intentionally not advertised as Bentley-Ottmann. Its worst-case complexity is
`O(n^2 + m^2)` including validation, where `m` is the number of atomic edges.

## Trace Phases

- `intersection_detection`
- `split_point_collection`
- `segment_splitting`
- `node_dedup`
- `edge_creation`
- `validation`

## CLI

```powershell
geokernel_demo --algorithm segment_arrangement --input examples/segment_arrangement.json --output out.json --trace --pretty
```

The output includes:

- `nodes`
- `edges`
- `split_segments`
- `intersections`
- `valid`
- `warnings`

## Current Limits

Intersection coordinates are still double constructions. The builder is designed for
small and medium correctness tests first; a production-scale sweep/overlay graph would
need more careful event ordering, coincident-edge policy, and performance work.
