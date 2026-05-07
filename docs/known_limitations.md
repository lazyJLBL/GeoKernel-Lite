# Known Limitations

GeoKernel-Lite is a computational geometry playground and portfolio project. It is not
a replacement for CGAL, GEOS, OpenCascade, or a production CAD/GIS kernel.

## Predicate Coverage

- `PredicateContext` is available across core algorithms, but not every numerical
  decision is exact.
- Exact predicates currently cover `orient2d` and `incircle` for finite IEEE-754
  `double` inputs.
- Polygon area, distance, projection, rectangle fitting, and line intersection
  coordinates are still computed in floating point.

## Topology

- Segment intersection reports pairwise `point` and `overlap` results. The separate
  segment arrangement builder can split segments into an arrangement-ready graph, but it
  is a correctness-first `O(n^2)` builder, not a production Bentley-Ottmann overlay
  engine.
- Polygon boolean now has data structures, normalization, validation, point
  classification, and a CLI skeleton. The actual overlay operation, result edge
  selection, ring tracing, and output construction are not implemented yet.
- Constrained Delaunay triangulation is a reserved include path only; it is not
  implemented yet.
- Polygon clipping is Sutherland-Hodgman against a convex clipper, not a general polygon
  boolean engine.

## Algorithms

- Half-plane intersection is implemented as iterative clipping against a finite
  bounding box for visualization stability.
- Delaunay triangulation remains experimental. Predicate-aware incircle classification
  and validation reports are wired in, but the triangulation does not yet provide
  industrial-grade neighbor structures, constrained edges, symbolic perturbation, or
  exact circumcenter construction.
- Closest pair uses robust duplicate detection through `PredicateContext`, but distances
  remain ordinary double distances.

## Testing

- Deterministic fuzz/property-style tests are included for sweep-line segment
  intersections and convex hull containment, but this is not exhaustive formal
  verification.
- JSON case catalogs focus on representative degenerate and boundary cases. They should
  grow as new algorithms are added.
