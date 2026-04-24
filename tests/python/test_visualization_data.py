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
