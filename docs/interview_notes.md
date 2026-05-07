# Interview Notes

GeoKernel-Lite is a C++17 computational geometry playground. It is meant to demonstrate
engineering judgment around robust predicates, degeneracies, traceable algorithms, and
honest validation. It is not a CGAL, GEOS, or OpenCascade replacement.

## Why EPS Fails

An EPS predicate turns a continuous determinant into three buckets: negative, zero, and
positive. That is convenient for programming contests, but it can collapse a genuinely
nonzero orientation or incircle determinant to zero. Once that happens, higher-level
algorithms may take a topologically different branch: a hull pops the wrong vertex, a
segment pair becomes collinear, an ear is rejected, or a Delaunay cavity is rebuilt
incorrectly.

EPS is still useful as an explicit mode because it shows the failure mode. The project
keeps it as `PredicateMode::Eps`, but defaults new predicate-aware paths to
`filtered_exact`.

## Filtered Exact Design

The predicate layer exposes:

- `orient2dEps`, `orient2dFiltered`, `orient2dExact`
- `incircleEps`, `incircleFiltered`, `incircleExact`
- `PredicateContext`, which routes algorithm decisions through a selected mode

The filtered exact path evaluates the determinant in floating point first and falls back
to exact dyadic arithmetic when the value is near the ambiguous region. This keeps common
cases fast while avoiding the worst EPS branch errors for finite `double` inputs.

## Exact Predicate vs Exact Construction

Exact predicates answer sign questions exactly: orientation is left/right/collinear;
incircle is inside/on/outside. They do not make all constructed coordinates exact.

The following are still double constructions in this project:

- segment intersection coordinates
- distances and areas
- projections and reflections
- minimum-area rectangle projections
- circumcenter-like derived values

This distinction is important. The project can make better topological decisions without
claiming to be a full exact-construction kernel.

## Algorithm Complexity

| Algorithm | Current implementation | Complexity |
| --- | --- | --- |
| Convex hull | Andrew monotone chain | `O(n log n)` |
| Convex diameter | rotating calipers on cyclic convex hull | `O(h)` |
| Minimum-area bounding rectangle | edge-direction scan over hull vertices | `O(h^2)` |
| Segment intersection search | ordered endpoint sweep plus oracle completion | sparse candidate pass plus worst-case `O(n^2 + k)` |
| Brute-force segment intersection | all segment pairs | `O(n^2)` |
| Segment arrangement | brute-force split builder | `O(n^2 + s log s)` plus validation |
| Closest pair | divide and conquer | `O(n log n)` |
| Polygon clipping | Sutherland-Hodgman against convex clipper | `O(nm)` |
| Ear clipping triangulation | ear clipping with validation | `O(n^3)` in the simple implementation |
| Delaunay | Bowyer-Watson prototype | experimental, validation-heavy |

`n` is input size, `h` is hull size, `k` is reported intersections, and `s` is the split
point count.

## Typical Degenerate Cases

1. Near-collinear orientation that EPS rounds to zero.
2. Near-cocircular incircle test that changes a Delaunay cavity.
3. Duplicate input points.
4. All points collinear.
5. Zero-length segments.
6. Endpoint-only segment touches.
7. Collinear overlapping segments.
8. Many segments sharing one event x-coordinate.
9. Polygon rings with duplicate closing vertices.
10. Bow-tie self-intersecting polygon input.

## Difference From CGAL/GEOS

CGAL provides mature exact kernels, arrangements, triangulations, and production-grade
topology. GEOS provides robust GIS geometry operations and prepared topology algorithms.
GeoKernel-Lite intentionally stays smaller: it is a portfolio project that makes the
predicate policy, degeneracy handling, trace design, and validation reports visible.

## Likely Follow-Up Questions

**Why keep EPS if it is dangerous?**

Because it is a useful baseline and failure demonstrator. The project can compare EPS,
filtered exact, and exact decisions on the same input.

**Does exact predicate mode make segment intersections exact?**

No. It makes the sign decisions exact for finite double inputs. Intersection coordinates
are still constructed with double arithmetic and are documented as such.

**Is the sweep-line a full Bentley-Ottmann implementation?**

Not yet. The current sweep uses ordered active neighbor checks and preserves the public
all-pairs contract with brute-force completion. A full implementation would add
intersection events and handle overlap topology more directly.

**Why is the minimum rectangle `O(h^2)`?**

The current function scans every hull edge direction and projects all hull vertices. That
is simple and reliable, but not the fully optimized rotating-calipers rectangle variant.

**What would make polygon boolean credible?**

A reusable arrangement/overlay graph, split edges, winding or fill-rule classification,
face tracing, containment trees for holes, validation reports, and fuzz tests. The
current project has the validation/data model foundation and a segment arrangement
builder, but general boolean overlay is still a future step.

## Future Route

1. Replace sweep oracle completion with a full Bentley-Ottmann event scheduler.
2. Build polygon boolean intersection on top of the segment arrangement graph.
3. Add ring tracing, containment trees, and holes/multipolygon support.
4. Extend Delaunay validation and add constrained/polygon-domain triangulation.
5. Expand fuzz/property testing and benchmark plots.
