# Robustness Failure Gallery

The robustness gallery is backed by JSON case catalogs in `tests/cases/`. The visualizer
loads all catalog files and lets predicate cases be executed from the robustness tab.

## Predicate Cases

`tests/cases/predicate_failures.json` adds deterministic `predicate_compare` cases for:

- near-collinear positive and negative orientations
- near-cocircular incircle classifications
- duplicate points
- large-coordinate and mixed-scale inputs
- small-scale incircle inputs

Each case records the input geometry and expected exact sign. The CLI result shows:

- EPS sign and determinant estimate
- filtered exact sign
- exact sign
- whether EPS differs from exact
- whether filtered matches exact

## Example

```powershell
geokernel_demo --algorithm predicate_compare --input case.json --output out.json --pretty
```

Input:

```json
{
  "input": {
    "predicate": "orient2d",
    "points": [[0, 0], [1, 0], [0.5, 1e-12]]
  }
}
```

Expected behavior:

- EPS reports `0`
- filtered exact reports `+1`
- exact reports `+1`

## Visualizer

The Streamlit robustness tab lists all boundary cases. Selecting a `predicate_compare`
case runs the C++ CLI, plots the points, and displays EPS, filtered, and exact signs.
