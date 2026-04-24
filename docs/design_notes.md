# Design Notes

## Header-Only Core

The first version keeps the C++ kernel header-only under
`core/include/geokernel/geokernel.hpp`. Wrapper headers preserve stable include paths
such as `geokernel/algorithm/convex_hull.hpp`, so the implementation can be split later
without breaking users.

## CLI + JSON Boundary

Python calls the C++ kernel through `geokernel_demo`. This keeps Streamlit independent
from C++ ABI and compiler details while still exercising the real algorithm
implementation.

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
