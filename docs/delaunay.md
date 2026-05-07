# Delaunay Triangulation

The Delaunay module remains experimental, but it now returns a validation report instead
of only a triangle list. This makes the prototype easier to reason about and safer to
use in demos.

## Algorithm

`delaunayTriangulation(points, options)` uses a simple Bowyer-Watson insertion
prototype:

1. remove duplicate points with `PredicateContext`
2. reject fewer-than-three and all-collinear unique inputs with warnings
3. build a large super-triangle
4. insert points one at a time
5. remove triangles whose circumcircle contains the inserted point
6. retriangulate the cavity boundary
7. discard triangles touching the super-triangle
8. validate the output

The cavity test uses `PredicateContext::incircle`, so `eps`, `filtered_exact`, and
`exact` modes are available.

## Validation Report

`DelaunayValidationReport` checks:

- no duplicate triangles
- all triangles are counter-clockwise
- no output triangle contains a non-input super-triangle vertex
- internal edges are shared by two triangles and hull edges by one
- empty-circle property
- triangle area sum covers the convex hull area

The CLI exposes the report under `result.validation` and mirrors key fields in
`summary`.

## Current Limits

This is not a production Delaunay triangulator. It does not maintain neighbor
structures, constrained edges, symbolic perturbation, or exact circumcenter
construction. Cocircular cases are accepted when the exact incircle predicate returns
zero, but the diagonal choice remains insertion-order dependent.
