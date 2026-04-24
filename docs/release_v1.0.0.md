# GeoKernel-Lite v1.0.0

GeoKernel-Lite v1.0.0 is the first portfolio-ready release of the 2D computational
geometry kernel and visual debugging platform.

## Highlights

- C++17 geometry kernel with reusable primitives and robust predicate helpers.
- Stable algorithm set: convex hull, rotating calipers, segment intersection search,
  half-plane intersection, closest pair, polygon clipping, and ear clipping
  triangulation.
- Experimental Bowyer-Watson Delaunay triangulation.
- CLI + JSON integration boundary for scripting and the Streamlit UI.
- Streamlit + Plotly visual debugging platform with trace-oriented geometry rendering.
- 56 boundary and degenerate test cases covering collinearity, overlaps, endpoint
  touches, duplicate points, tiny distances, orientation normalization, empty
  intersections, and degenerate polygon outputs.
- Windows GitHub Actions workflow for C++ build/tests and Python tests.

## Validation

- Latest remote `windows` GitHub Actions run was green before release preparation.
- Local Python tests pass with `python -m pytest`.
- Header-only C++ benchmark target compiles with MSYS2 `g++` in the current Windows
  environment.

## Known Limitations

- The first version is Windows-first; Linux and macOS CI are intentionally left for a
  later release.
- Delaunay triangulation is marked experimental and is not part of the same stability
  tier as the seven core algorithms.
- Half-plane intersection clips unbounded regions by a configurable bounding box for
  visualization.
- The Python layer calls the C++ kernel through CLI + JSON rather than pybind11
  bindings.
