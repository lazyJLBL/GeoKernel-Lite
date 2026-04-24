from __future__ import annotations

from typing import Any

import plotly.graph_objects as go


def _xy(points: list[list[float]]) -> tuple[list[float], list[float]]:
    return [p[0] for p in points], [p[1] for p in points]


def _closed(points: list[list[float]]) -> list[list[float]]:
    if len(points) > 1 and points[0] != points[-1]:
        return [*points, points[0]]
    return points


def add_points(fig: go.Figure, points: list[list[float]], name: str = "points", color: str = "#1f77b4") -> None:
    if not points:
        return
    x, y = _xy(points)
    fig.add_trace(go.Scatter(x=x, y=y, mode="markers", name=name, marker={"size": 8, "color": color}))


def add_segments(fig: go.Figure, segments: list[list[list[float]]], name: str = "segments", color: str = "#636363") -> None:
    for index, segment in enumerate(segments):
        x, y = _xy(segment)
        fig.add_trace(go.Scatter(x=x, y=y, mode="lines+markers", name=f"{name} {index}", line={"color": color, "width": 2}))


def add_polygon(fig: go.Figure, points: list[list[float]], name: str, color: str, fill: str = "toself") -> None:
    if not points:
        return
    x, y = _xy(_closed(points))
    fig.add_trace(
        go.Scatter(
            x=x,
            y=y,
            mode="lines+markers",
            name=name,
            fill=fill,
            line={"color": color, "width": 2},
            marker={"size": 7, "color": color},
            opacity=0.78,
        )
    )


def empty_figure(title: str = "GeoKernel-Lite") -> go.Figure:
    fig = go.Figure()
    fig.update_layout(
        title=title,
        template="plotly_white",
        height=620,
        margin={"l": 24, "r": 24, "t": 48, "b": 24},
        xaxis={"scaleanchor": "y", "scaleratio": 1, "zeroline": True, "showgrid": True},
        yaxis={"zeroline": True, "showgrid": True},
        legend={"orientation": "h", "y": -0.12},
    )
    return fig


def figure_for_result(algorithm: str, payload: dict[str, Any], output: dict[str, Any] | None = None, step: int | None = None) -> go.Figure:
    fig = empty_figure(algorithm.replace("_", " ").title())
    input_data = payload.get("input", payload)
    result = (output or {}).get("result", {})

    if "points" in input_data:
        add_points(fig, input_data["points"], "input points", "#0f6b78")
    if "segments" in input_data:
        add_segments(fig, input_data["segments"], "input segment", "#525252")
    if "subject" in input_data:
        add_polygon(fig, input_data["subject"], "subject", "#0f6b78", fill="none")
    if "clipper" in input_data:
        add_polygon(fig, input_data["clipper"], "clipper", "#a23e2b")
    if "polygon" in input_data:
        add_polygon(fig, input_data["polygon"], "input polygon", "#0f6b78", fill="none")

    if result.get("hull"):
        add_polygon(fig, result["hull"], "convex hull", "#c43b3b")
    if result.get("polygon"):
        add_polygon(fig, result["polygon"], "result polygon", "#2f855a")
    if result.get("diameter"):
        add_segments(fig, [[result["diameter"]["p1"], result["diameter"]["p2"]]], "diameter", "#c43b3b")
    if result.get("minimum_area_rectangle"):
        add_polygon(fig, result["minimum_area_rectangle"]["corners"], "minimum rectangle", "#6b46c1", fill="none")
    if result.get("p1") and result.get("p2"):
        add_segments(fig, [[result["p1"], result["p2"]]], "closest pair", "#c43b3b")
    if result.get("triangles"):
        for index, tri in enumerate(result["triangles"]):
            add_polygon(fig, tri, f"triangle {index}", "#2f855a", fill="none")

    trace = (output or {}).get("trace", [])
    if step is not None and trace:
        step = max(0, min(step, len(trace) - 1))
        geometry = trace[step].get("geometry", {})
        add_points(fig, geometry.get("points", []), "trace points", "#d97706")
        add_segments(fig, geometry.get("segments", []), "trace segment", "#d97706")
        for poly in geometry.get("polygons", []):
            add_polygon(fig, poly, "trace polygon", "#d97706", fill="none")

    return fig
