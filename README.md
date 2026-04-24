# GeoKernel-Lite: 2D Computational Geometry Kernel and Visual Debugging Platform

[![Windows CI](https://github.com/lazyJLBL/GeoKernel-Lite/actions/workflows/windows.yml/badge.svg)](https://github.com/lazyJLBL/GeoKernel-Lite/actions/workflows/windows.yml)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C)
![Python](https://img.shields.io/badge/Python-3.12-3776AB)
![License](https://img.shields.io/badge/License-MIT-green)

A lightweight C++ 2D computational geometry kernel with robust predicates, classic
geometry algorithms, edge-case tests, and Streamlit visual debugging.

GeoKernel-Lite 是一个面向 CAD/CAX、GIS、图形算法和工业软件研发场景的二维计算几何算法内核项目。核心算法使用 C++17 实现，可视化调试平台使用 Python +
Streamlit + Plotly 构建，用于展示算法结果和中间 trace。

## Visual Debugging Screenshots

| Convex Hull                                                           | Segment Intersection                                                                    | Polygon Clipping                                                                |
| --------------------------------------------------------------------- | --------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------- |
| ![Convex hull Streamlit screenshot](assets/streamlit_convex_hull.png) | ![Segment intersection Streamlit screenshot](assets/streamlit_segment_intersection.png) | ![Polygon clipping Streamlit screenshot](assets/streamlit_polygon_clipping.png) |

| Half-Plane Intersection                                                                       | Closest Pair                                                            | Triangulation                                                             |
| --------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------- | ------------------------------------------------------------------------- |
| ![Half-plane intersection Streamlit screenshot](assets/streamlit_half_plane_intersection.png) | ![Closest pair Streamlit screenshot](assets/streamlit_closest_pair.png) | ![Triangulation Streamlit screenshot](assets/streamlit_triangulation.png) |

## Project Highlights

- C++17 geometry kernel with `Point2D`, `Segment2D`, `Polygon2D`, `HalfPlane2D`, and
  related primitives.
- Robust geometric predicates with EPS-aware equality, strict sortable ordering,
  boundary-aware classifications, and polygon normalization.
- Classic geometry algorithms: convex hull, rotating calipers, segment intersection
  search, half-plane intersection, closest pair, polygon clipping, and ear clipping
  triangulation.
- Experimental Bowyer-Watson Delaunay triangulation for visualization and future
  expansion.
- CLI + JSON boundary between the C++ kernel and Python UI, keeping the algorithm core
  independent from Streamlit.
- 56 degenerate and boundary cases covering collinearity, overlaps, endpoint touches,
  duplicate points, tiny distances, orientation issues, empty intersections, and
  degenerate polygon results.

## Features

### Geometry Primitives

- Point, vector, line, segment, circle, polygon, half-plane, box, and triangle types.
- Dot product, cross product, orientation test, distance, projection, and reflection
  helpers.
- Segment intersection with `None`, `Point`, and `Overlap` classification.
- Point-in-polygon with `Outside`, `Inside`, and `OnBoundary` classification.
- Polygon area, orientation checks, and counter-clockwise normalization.

### Algorithms

- Andrew convex hull and Graham-compatible entry point.
- Rotating calipers for convex diameter and minimum-area bounding rectangle.
- Segment intersection search with classified point and overlap results.
- Half-plane intersection clipped by a configurable visualization bounding box.
- Divide-and-conquer closest pair of points.
- Sutherland-Hodgman polygon clipping.
- Ear clipping triangulation with area verification.
- Experimental Delaunay triangulation.

### Visualization

- Streamlit tabs for each algorithm.
- Plotly geometry layers for points, segments, polygons, hulls, clipping windows,
  closest-pair links, and triangulation results.
- Trace step slider for intermediate algorithm states.
- Robustness case browser backed by the same JSON cases used in tests.

## Quick Start

Install dependencies:

```powershell
python -m pip install -r requirements.txt
```

Build and test the C++ targets:

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run Python tests:

```powershell
python -m pytest
```

Launch the visualizer:

```powershell
streamlit run visualizer/app.py
```

## CLI Demo

The C++ command-line tool uses a stable JSON envelope so it can be called from scripts,
tests, or the Streamlit UI.

```powershell
.\build\geokernel_demo.exe --algorithm convex_hull --input examples\convex_hull.json --output out.json --trace --pretty
```

Successful responses use this shape:

```json
{
  "status": "ok",
  "summary": {},
  "result": {},
  "trace": [],
  "warnings": []
}
```

Supported algorithm names:

- `convex_hull`
- `rotating_calipers`
- `segment_intersection`
- `half_plane_intersection`
- `polygon_clipping`
- `closest_pair`
- `triangulation`
- `delaunay`

## Project Structure

```text
GeoKernel-Lite/
|-- core/                  # Header-only C++ geometry kernel
|-- apps/                  # geokernel_demo CLI
|-- python/                # Python case loader, runner, visualization adapter
|-- visualizer/            # Streamlit + Plotly app
|-- tests/                 # C++ tests, Python tests, boundary cases
|-- docs/                  # Algorithms, robustness, API, design notes
|-- examples/              # Reproducible JSON examples
|-- benchmarks/            # Simple C++ benchmark target
|-- assets/                # README screenshots
`-- CMakeLists.txt
```

## Robustness Focus

GeoKernel-Lite treats robustness as a first-class API concern. Important geometric
predicates do not collapse ambiguous states into `bool`: segment intersection reports
point vs overlap, point-in-polygon reports boundary hits, clipping reports empty vs
polygon vs degenerate, and half-plane intersection reports bounded, unbounded, empty, or
degenerate results.

See [docs/robustness.md](docs/robustness.md) and
[docs/edge_cases_report.md](docs/edge_cases_report.md) for the detailed policy and
boundary-case catalog.

## Release Status

`v1.0.0` is the first portfolio-ready release target. The core seven algorithms are
treated as stable v1 functionality; Delaunay remains explicitly experimental.
