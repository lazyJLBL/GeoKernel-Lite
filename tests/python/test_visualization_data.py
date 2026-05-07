from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "python"))

from geokernel_lite.visualization import figure_for_result


def test_visualization_builds_convex_hull_figure():
    payload = {"input": {"points": [[0, 0], [1, 0], [0, 1]]}}
    output = {
        "result": {"hull": [[0, 0], [1, 0], [0, 1]]},
        "trace": [
            {
                "step_index": 0,
                "phase": "complete",
                "message": "done",
                "geometry": {"points": [[0, 0]], "segments": [], "polygons": []},
                "metrics": {},
            }
        ],
    }
    fig = figure_for_result("convex_hull", payload, output, step=0)
    assert len(fig.data) >= 2


def test_visualization_builds_segment_figure():
    payload = {"input": {"segments": [[[0, 0], [1, 1]], [[0, 1], [1, 0]]]}}
    fig = figure_for_result("segment_intersection", payload)
    assert len(fig.data) == 2


def test_visualization_builds_arrangement_figure():
    payload = {"input": {"segments": [[[0, 0], [2, 2]], [[0, 2], [2, 0]]]}}
    output = {
        "result": {
            "nodes": [{"point": [0, 0]}, {"point": [1, 1]}, {"point": [2, 2]}],
            "edges": [
                {"segment": [[0, 0], [1, 1]]},
                {"segment": [[1, 1], [2, 2]]},
            ],
        },
        "trace": [],
    }
    fig = figure_for_result("segment_arrangement", payload, output)
    assert len(fig.data) >= 5


def test_visualization_builds_predicate_compare_figure():
    payload = {"input": {"predicate": "orient2d", "points": [[0, 0], [1, 0], [0.5, 1e-12]]}}
    output = {
        "result": {
            "predicate": "orient2d",
            "eps_differs_from_exact": True,
            "filtered_matches_exact": True,
        },
        "trace": [],
    }
    fig = figure_for_result("predicate_compare", payload, output)
    assert len(fig.data) == 1


def test_visualization_builds_polygon_boolean_figure():
    payload = {
        "input": {
            "subject": {"polygons": [{"outer": [[0, 0], [2, 0], [2, 2], [0, 2]], "holes": []}]},
            "clip": {"polygons": [{"outer": [[1, 1], [3, 1], [3, 3], [1, 3]], "holes": []}]},
        }
    }
    output = {"result": {"multipolygon": {"polygons": []}}, "trace": []}
    fig = figure_for_result("polygon_boolean", payload, output)
    assert len(fig.data) == 2
