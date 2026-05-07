# Design Notes

## Header-Only Core

The first version kept most C++ implementation in
`core/include/geokernel/geokernel.hpp`. The current tree now exposes compatibility
module headers under:

- `geokernel/core/`
- `geokernel/trace/`
- `geokernel/io/`
- `geokernel/algorithm/`

`geokernel.hpp` remains the umbrella header and still includes the public module paths.
Most algorithm bodies intentionally remain in the umbrella header for now because the
types, helpers, trace structs, and algorithms are still tightly coupled. This keeps old
include paths working while creating stable landing zones for future extraction.

The `arrangement` include path now exposes the correctness-first segment arrangement
builder. `polygon_boolean` exposes data structures, normalization, validation, and a
CLI skeleton. `constrained_delaunay` remains a reserved public include path only.

TODO: move implementation blocks out of `geokernel.hpp` once each module has narrow
dependencies and dedicated tests.

## Predicate Context

`PredicateContext` centralizes predicate policy:

- `eps`
- `filtered_exact`
- `exact`

Algorithms accept this through `AlgorithmOptions::predicates` where possible. Existing
function names remain available, and direct legacy calls such as `segmentIntersection(a,
b)` keep their compatibility behavior.

## CLI + JSON Boundary

Python calls the C++ kernel through `geokernel_demo`. This keeps Streamlit independent
from C++ ABI and compiler details while still exercising the real algorithm
implementation.

The CLI accepts optional `predicate_mode` and `predicate_eps` fields for predicate-aware
algorithms. JSON serialization remains in `apps/geokernel_demo.cpp`; the header-only
core does not depend on `nlohmann_json`.

## Trace Shape

Every trace step uses the same structure:

- `step_index`
- `phase`
- `message`
- `geometry.points`
- `geometry.segments`
- `geometry.polygons`
- `metrics`

This lets the visualizer render very different algorithms with one adapter.

## Stability Tiers

The core seven algorithms are treated as stable v1 functionality. Delaunay is shipped
for experimentation and visualization, but documented separately so it does not weaken
the reliability claim of the main kernel.
