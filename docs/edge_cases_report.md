# Edge Cases Report

The boundary-case catalog lives in `tests/cases/catalog.json` and currently contains 56 cases.

| Module | Covered cases |
|---|---|
| Convex hull | empty input, single point, two points, duplicates, all-collinear drop/keep, interior points |
| Rotating calipers | rectangle, single point, two points, triangle, rotated diamond, duplicates, noisy cloud |
| Segment intersection | crossing, endpoint touch, partial overlap, full overlap, parallel disjoint, zero-length segment, near-collinear precision |
| Half-plane intersection | bounded square, empty opposite planes, unbounded strip, triangle, duplicate plane, large coordinates, degenerate line |
| Polygon clipping | inside, outside, partial overlap, vertex on edge, shared edge, degenerate line, concave subject |
| Closest pair | two points, duplicate points, tiny distance, large coordinates, grid, negative coordinates, insufficient points |
| Triangulation | triangle, square, concave polygon, clockwise input, repeated vertex, collinear vertex, self-intersection |
| Delaunay | triangle, square, duplicates, interior point, collinear input, large coordinates, random cloud |

Representative segment case:

```json
{
  "name": "segment_partial_overlap",
  "algorithm": "segment_intersection",
  "input": {
    "segments": [[[0, 0], [4, 0]], [[2, 0], [6, 0]]]
  },
  "expected": {
    "type": "overlap"
  }
}
```
