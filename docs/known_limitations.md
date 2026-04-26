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

- Segment intersection reports pairwise `point` and `overlap` results. It does not build
  a full planar arrangement or merge overlapping topology.
- Polygon boolean operations, arrangement construction, and constrained Delaunay
  triangulation are reserved include paths only; they are not implemented yet.
- Polygon clipping is Sutherland-Hodgman against a convex clipper, not a general polygon
  boolean engine.

## Algorithms

- Half-plane intersection is implemented as iterative clipping against a finite
  bounding box for visualization stability.
- Delaunay triangulation remains experimental. Predicate-aware incircle classification
  is wired in, but the triangulation does not yet provide industrial-grade topology
  validation, neighbor structures, or constrained edges.
- Closest pair uses robust duplicate detection through `PredicateContext`, but distances
  remain ordinary double distances.

## Testing

- Deterministic fuzz/property-style tests are included for sweep-line segment
  intersections and convex hull containment, but this is not exhaustive formal
  verification.
- JSON case catalogs focus on representative degenerate and boundary cases. They should
  grow as new algorithms are added.
