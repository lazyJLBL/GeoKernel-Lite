# Visualization Guide

Run the app with:

```powershell
streamlit run visualizer/app.py
```

Each algorithm tab loads sample cases from `tests/cases/catalog.json`. The input editor accepts the same JSON payload used by the C++ CLI. When `geokernel_demo` exists under `build/`, the app runs the C++ executable and renders the returned `result` and `trace`.

The trace slider overlays intermediate geometry:

- points from the current algorithm step
- candidate or active segments
- intermediate polygons
- per-step metrics in a table

The `Robustness Cases` tab lists the shared JSON boundary catalog used by tests and examples.
