# Predicates

GeoKernel-Lite separates legacy EPS predicates from the P0 exact predicate layer.

## API

- `orient2dEps(a, b, c)`
- `orient2dFiltered(a, b, c)`
- `orient2dExact(a, b, c)`
- `incircleEps(a, b, c, d)`
- `incircleFiltered(a, b, c, d)`
- `incircleExact(a, b, c, d)`
- `compareOrient2d(...)`
- `compareIncircle(...)`

`PredicateMode` selects the policy used by predicate-aware algorithms:

- `Eps`
- `FilteredExact`
- `Exact`

`PredicateContext` packages the selected mode and EPS value and is available through
`AlgorithmOptions::predicates`.

## Semantics

`Eps` uses the project-wide `1e-9` tolerance. This is useful for legacy behavior and
visual demos, but it can collapse a nonzero determinant to zero.

`FilteredExact` first evaluates the determinant in double precision. If the sign is
clearly separated from zero, that sign is returned. Otherwise it falls back to exact
dyadic arithmetic over finite IEEE-754 `double` inputs.

`Exact` always uses the exact fallback. Non-finite inputs are rejected with
`std::invalid_argument`.

## Why It Matters

For nearly collinear points:

```text
a = (0, 0)
b = (1, 0)
c = (0.5, 1e-12)
```

EPS reports `0`, while filtered exact and exact report `+1`.

For nearly cocircular points:

```text
a = (0, 0)
b = (1, 0)
c = (0, 1)
d = (1, 0.999999999999)
```

EPS reports `0`, while filtered exact and exact report `+1`.

## Current Limits

The exact predicate layer currently covers `orient2d` and `incircle`. Existing polygon,
half-plane, triangulation, and Delaunay code still use EPS predicates except where a
predicate-aware API explicitly accepts `PredicateMode`.
