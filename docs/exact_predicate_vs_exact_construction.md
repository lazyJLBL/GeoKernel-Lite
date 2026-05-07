# Exact Predicate vs Exact Construction

GeoKernel-Lite currently provides exact sign classification for selected predicates,
not full exact geometry.

## Exact Predicate

An exact predicate answers a discrete question such as:

- are three points clockwise, counter-clockwise, or collinear?
- is a point inside, outside, or on the circumcircle of a triangle?

`orient2dExact` and `incircleExact` evaluate those signs with dependency-free dyadic
arithmetic over finite IEEE-754 `double` inputs. `filtered_exact` first tries a fast
double estimate, then falls back to the exact path when the sign is numerically close
to zero.

## Exact Construction

An exact construction would compute new geometry exactly, for example:

- line intersection coordinates
- polygon clipping vertices
- circumcenters
- distances and projections
- polygon area as an exact value

GeoKernel-Lite still computes those constructions with ordinary `double` arithmetic.
This means an exact orientation predicate can decide that two segments cross, while the
reported intersection coordinate is still a floating-point construction.

## Project Policy

The code and docs should use precise wording:

- say "exact predicate" for `orient2d` and `incircle` signs
- do not say "exact geometry kernel"
- do not imply that polygon boolean output coordinates are exact
- document any algorithm whose topology decision is predicate-aware but whose output
  coordinates are double-based

This distinction is central to the project: robust topology decisions are valuable, but
they are not the same as a CGAL-style exact-construction kernel.
