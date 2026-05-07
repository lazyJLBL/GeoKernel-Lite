from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "python"))

from geokernel_lite.runner import GeoKernelRunner
from geokernel_lite.runner import locate_demo_executable

import pytest


def test_runner_command_contract(tmp_path):
    executable = tmp_path / "geokernel_demo.exe"
    executable.write_text("", encoding="utf-8")
    runner = GeoKernelRunner(executable)
    command = runner.command("convex_hull", tmp_path / "in.json", tmp_path / "out.json", trace=True, pretty=True)
    assert "--algorithm" in command
    assert "convex_hull" in command
    assert "--trace" in command
    assert "--pretty" in command


def test_predicate_compare_cli_contract():
    executable = locate_demo_executable()
    if executable is None:
        pytest.skip("geokernel_demo executable is not built")

    output = GeoKernelRunner(executable).run(
        "predicate_compare",
        {"predicate": "orient2d", "points": [[0, 0], [1, 0], [0.5, 1e-12]]},
        trace=False,
    )

    assert output["status"] == "ok"
    assert output["summary"]["algorithm"] == "predicate_compare"
    assert output["result"]["eps_differs_from_exact"] is True
    assert output["result"]["filtered_matches_exact"] is True


def test_segment_intersection_accepts_predicate_mode():
    executable = locate_demo_executable()
    if executable is None:
        pytest.skip("geokernel_demo executable is not built")

    output = GeoKernelRunner(executable).run(
        "segment_intersection",
        {
            "predicate_mode": "filtered_exact",
            "segments": [[[0, 0], [2, 0]], [[1, 1e-10], [3, 1e-10]]],
        },
        trace=False,
    )

    assert output["status"] == "ok"
    assert output["summary"]["predicate_mode"] == "filtered_exact"
    assert output["summary"]["predicate_eps"] == 1e-9
    assert output["result"]["pairs"] == []


def test_segment_arrangement_cli_splits_crossing_segments():
    executable = locate_demo_executable()
    if executable is None:
        pytest.skip("geokernel_demo executable is not built")

    output = GeoKernelRunner(executable).run(
        "segment_arrangement",
        {
            "predicate_mode": "filtered_exact",
            "segments": [[[0, 0], [2, 2]], [[0, 2], [2, 0]]],
        },
        trace=True,
    )

    assert output["status"] == "ok"
    assert output["summary"]["algorithm"] == "segment_arrangement"
    assert output["summary"]["valid"] is True
    assert output["summary"]["node_count"] == 5
    assert output["summary"]["edge_count"] == 4
    assert output["result"]["split_segments"][0]["atomic_segments"]
    assert output["trace"]


def test_polygon_boolean_cli_validates_and_reports_skeleton():
    executable = locate_demo_executable()
    if executable is None:
        pytest.skip("geokernel_demo executable is not built")

    output = GeoKernelRunner(executable).run(
        "polygon_boolean",
        {
            "operation": "union",
            "fill_rule": "even_odd",
            "predicate_mode": "filtered_exact",
            "subject": {
                "polygons": [
                    {"outer": [[0, 0], [4, 0], [4, 4], [0, 4], [0, 0]], "holes": []}
                ]
            },
            "clip": {
                "polygons": [
                    {"outer": [[2, 2], [6, 2], [6, 6], [2, 6]], "holes": []}
                ]
            },
        },
        trace=True,
    )

    assert output["status"] == "ok"
    assert output["summary"]["algorithm"] == "polygon_boolean"
    assert output["summary"]["operation"] == "union"
    assert output["summary"]["predicate_mode"] == "filtered_exact"
    assert output["summary"]["predicate_eps"] == 1e-9
    assert output["summary"]["valid_input"] is True
    assert output["summary"]["valid_output"] is False
    assert output["result"]["input_validation"]["valid"] is True
    assert "polygon_boolean_operation_not_implemented" in output["warnings"]
    assert output["trace"]


def test_polygon_boolean_cli_returns_structured_validation_error():
    executable = locate_demo_executable()
    if executable is None:
        pytest.skip("geokernel_demo executable is not built")

    output = GeoKernelRunner(executable).run(
        "polygon_boolean",
        {
            "operation": "intersection",
            "subject": {
                "polygons": [
                    {"outer": [[0, 0], [2, 2], [0, 2], [2, 0]], "holes": []}
                ]
            },
            "clip": {
                "polygons": [
                    {"outer": [[0, 0], [1, 0], [1, 1], [0, 1]], "holes": []}
                ]
            },
        },
        trace=False,
    )

    assert output["status"] == "error"
    assert output["summary"]["valid_input"] is False
    assert output["result"]["input_validation"]["valid"] is False
    assert any("ring_self_intersects" in item for item in output["result"]["input_validation"]["errors"])


def test_delaunay_cli_reports_validation():
    executable = locate_demo_executable()
    if executable is None:
        pytest.skip("geokernel_demo executable is not built")

    output = GeoKernelRunner(executable).run(
        "delaunay",
        {"predicate_mode": "filtered_exact", "points": [[0, 0], [1, 0], [0, 1], [1, 1]]},
        trace=False,
    )

    assert output["status"] == "ok"
    assert output["summary"]["algorithm"] == "delaunay"
    assert output["summary"]["valid"] is True
    assert output["summary"]["edge_count"] == output["result"]["validation"]["edge_count"]
    assert output["result"]["validation"]["empty_circle_property"] is True
