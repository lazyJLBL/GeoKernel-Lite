# Edge Cases Report

The boundary-case catalog lives in `tests/cases/*.json` and currently contains 100+
cases. The same catalogs are used by the visualizer's robustness tab and Python tests,
so documentation, demos, and test data stay aligned.

## Coverage Summary

| Module                  | Count | Main risks covered                                                                                      |
| ----------------------- | ----: | ------------------------------------------------------------------------------------------------------- |
| Convex hull             |     7 | empty input, small input, duplicates, all-collinear sets, interior points                               |
| Rotating calipers       |     7 | degenerate hulls, rectangles, rotated hulls, duplicate points                                           |
| Segment intersection    |     8 | crossing, endpoint touch, overlap, parallel disjoint, zero-length, near-collinear, predicate modes      |
| Half-plane intersection |     7 | bounded result, empty result, unbounded clipping, duplicate planes, large coordinates, degenerate line  |
| Polygon clipping        |     7 | inside, outside, partial overlap, boundary vertex, shared edge, degenerate line, concave subject        |
| Closest pair            |     7 | duplicate points, tiny distance, large coordinates, grid ties, negative coordinates, insufficient input |
| Triangulation           |     7 | convex, concave, clockwise input, duplicate vertex, collinear vertex, self-intersection                 |
| Delaunay experimental   |     7 | triangle, square, duplicates, interior point, collinear input, large coordinates, random cloud          |
| Predicate comparison    |    50 | near-collinear orientation, near-cocircular incircle, duplicate points, mixed coordinate scales         |

## Convex Hull Cases

| Case                        | Input feature                              | Expected behavior               |
| --------------------------- | ------------------------------------------ | ------------------------------- |
| `convex_empty`              | no points                                  | empty hull                      |
| `convex_single_point`       | one point                                  | one hull vertex                 |
| `convex_two_points`         | two points                                 | two hull vertices               |
| `convex_duplicates`         | repeated points                            | duplicates removed before hull  |
| `convex_all_collinear_drop` | all points collinear                       | endpoints only, warning emitted |
| `convex_all_collinear_keep` | all points collinear with `keep_collinear` | all boundary points retained    |
| `convex_square_with_inner`  | square plus interior point                 | interior point removed          |

## Rotating Calipers Cases

| Case                       | Input feature          | Expected behavior                         |
| -------------------------- | ---------------------- | ----------------------------------------- |
| `calipers_rectangle`       | axis-aligned rectangle | rectangle area preserved                  |
| `calipers_single`          | one point              | zero-area rectangle                       |
| `calipers_two_points`      | line segment hull      | diameter equals segment length            |
| `calipers_triangle`        | triangle hull          | nonzero candidate rectangle               |
| `calipers_rotated`         | diamond-shaped hull    | angle-aware rectangle                     |
| `calipers_with_duplicates` | duplicate vertices     | hull preprocessing removes duplicates     |
| `calipers_wide_cloud`      | interior noise points  | hull-based result ignores interior points |

## Segment Intersection Cases

| Case                             | Input feature                          | Expected behavior              |
| -------------------------------- | -------------------------------------- | ------------------------------ |
| `segment_cross`                  | two interior-crossing segments         | point intersection             |
| `segment_endpoint_touch`         | shared endpoint                        | point intersection at endpoint |
| `segment_partial_overlap`        | collinear partial overlap              | overlap segment                |
| `segment_full_overlap`           | identical segments                     | overlap segment                |
| `segment_parallel_disjoint`      | parallel separate segments             | no intersection                |
| `segment_zero_length_on_segment` | zero-length segment on another segment | point intersection             |
| `segment_nearly_collinear`       | nearly collinear under EPS             | stable overlap classification  |
| `segment_nearly_collinear_filtered_exact` | near-parallel under exact predicates | no false overlap               |

Representative JSON:

```json
{
  "name": "segment_partial_overlap",
  "algorithm": "segment_intersection",
  "input": {
    "segments": [
      [
        [0, 0],
        [4, 0]
      ],
      [
        [2, 0],
        [6, 0]
      ]
    ]
  },
  "expected": {
    "type": "overlap"
  }
}
```

## Half-Plane Intersection Cases

| Case                        | Input feature                     | Expected behavior                   |
| --------------------------- | --------------------------------- | ----------------------------------- |
| `halfplane_unit_square`     | four bounding half-planes         | bounded polygon with area 1         |
| `halfplane_empty_opposite`  | incompatible opposite half-planes | empty result                        |
| `halfplane_unbounded_strip` | finite-width infinite strip       | unbounded status after box clipping |
| `halfplane_triangle`        | three half-planes                 | bounded triangle                    |
| `halfplane_duplicate`       | repeated half-plane               | unchanged bounded result            |
| `halfplane_large_box`       | large coordinates                 | stable bounded polygon              |
| `halfplane_degenerate_line` | area collapses to a line          | degenerate result                   |

## Polygon Clipping Cases

| Case                   | Input feature                   | Expected behavior                        |
| ---------------------- | ------------------------------- | ---------------------------------------- |
| `clip_inside`          | subject fully inside clipper    | original subject retained                |
| `clip_outside`         | subject fully outside clipper   | empty result                             |
| `clip_partial`         | partial overlap                 | clipped polygon                          |
| `clip_vertex_on_edge`  | subject vertex on clip edge     | boundary handled without duplicate noise |
| `clip_shared_edge`     | overlapping clip edge           | shared edge retained                     |
| `clip_degenerate_line` | very thin polygon               | degenerate status                        |
| `clip_concave_subject` | concave subject, convex clipper | valid clipped polygon                    |

## Closest Pair Cases

| Case                        | Input feature                | Expected behavior             |
| --------------------------- | ---------------------------- | ----------------------------- |
| `closest_two_points`        | exactly two points           | distance between those points |
| `closest_duplicate`         | repeated point               | zero distance                 |
| `closest_small_distance`    | tiny separation              | precision retained            |
| `closest_large_coordinates` | large coordinate values      | stable distance               |
| `closest_grid`              | multiple equal nearest pairs | valid unit distance           |
| `closest_negative`          | negative coordinates         | normal nearest pair           |
| `closest_insufficient`      | one point                    | invalid result with warning   |

## Triangulation Cases

| Case                           | Input feature            | Expected behavior        |
| ------------------------------ | ------------------------ | ------------------------ |
| `triangulate_triangle`         | already a triangle       | one output triangle      |
| `triangulate_square`           | convex quadrilateral     | two triangles            |
| `triangulate_concave_l`        | concave L shape          | valid ear clipping       |
| `triangulate_clockwise`        | clockwise input          | orientation normalized   |
| `triangulate_repeated_vertex`  | repeated adjacent vertex | duplicate removed        |
| `triangulate_collinear_vertex` | collinear middle vertex  | redundant vertex removed |
| `triangulate_self_intersect`   | bow-tie polygon          | rejected as invalid      |

The triangulation validator compares total triangle area with normalized polygon area.
This catches missing ears, overlapping triangles, and incorrect orientation handling.

## Experimental Delaunay Cases

| Case                         | Input feature              | Expected behavior                  |
| ---------------------------- | -------------------------- | ---------------------------------- |
| `delaunay_triangle`          | three points               | one experimental triangle          |
| `delaunay_square`            | square points              | diagonal triangulation             |
| `delaunay_duplicate_points`  | repeated point             | duplicates removed                 |
| `delaunay_five_points`       | interior point             | multiple triangles                 |
| `delaunay_collinear`         | collinear input            | documented experimental degeneracy |
| `delaunay_large_coordinates` | large values               | accepted by prototype              |
| `delaunay_random_cloud`      | small non-degenerate cloud | experimental triangulation         |

## Predicate Comparison Cases

`tests/cases/predicate_failures.json` covers EPS-vs-exact disagreement for
`orient2d` and `incircle`. These cases are executable through the `predicate_compare`
CLI path and the visualizer robustness tab.

## Acceptance Criteria

Every case must include:

- `name`
- `algorithm`
- `description`
- `input`
- `expected`
- `tags`

Python tests verify this structure and enforce the minimum case count. C++ tests
separately validate representative numerical behavior and algorithm invariants.
