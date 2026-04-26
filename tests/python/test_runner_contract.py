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
    assert output["result"]["pairs"] == []
