# Polygon Boolean

Polygon boolean support is currently in the infrastructure stage. The project now has
the topology-facing data model, normalization, validation, JSON contract, and CLI
skeleton needed by a later arrangement/overlay implementation.

## Data Model

- `Ring2D`
- `PolygonWithHoles2D`
- `MultiPolygon2D`
- `FillRule`
- `BooleanOp`
- `RingOrientation`
- `BoundaryLocation`

Outer rings normalize to counter-clockwise orientation. Holes normalize to clockwise
orientation. Duplicate closing points, consecutive duplicates, and collinear redundant
vertices are removed before validation.

## Validation

The validator rejects:

- rings with fewer than three vertices after normalization
- zero-area rings
- self-intersecting rings
- holes outside the outer ring
- holes touching or crossing the outer boundary
- overlapping or nested holes
- overlapping multipolygon components

Point classification is exposed for rings, polygons with holes, and multipolygons with
explicit `inside`, `outside`, and `on_boundary` states.

## CLI Contract

```json
{
  "algorithm": "polygon_boolean",
  "input": {
    "operation": "union",
    "fill_rule": "even_odd",
    "predicate_mode": "filtered_exact",
    "predicate_eps": 1e-9,
    "subject": {
      "polygons": [
        {
          "outer": [[0, 0], [4, 0], [4, 4], [0, 4]],
          "holes": []
        }
      ]
    },
    "clip": {
      "polygons": [
        {
          "outer": [[2, 2], [6, 2], [6, 6], [2, 6]],
          "holes": []
        }
      ]
    }
  }
}
```

The current CLI validates and normalizes the input, then returns
`polygon_boolean_operation_not_implemented`. It intentionally does not claim to compute
union, intersection, difference, or xor yet.

## Current Limits

The boolean overlay graph, edge classification, ring tracing, and output construction
are not implemented in this stage. The next step is a correctness-first segment
arrangement builder that all future polygon boolean work will reuse.
