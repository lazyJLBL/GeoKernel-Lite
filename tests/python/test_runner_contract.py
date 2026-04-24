from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "python"))

from geokernel_lite.runner import GeoKernelRunner


def test_runner_command_contract(tmp_path):
    executable = tmp_path / "geokernel_demo.exe"
    executable.write_text("", encoding="utf-8")
    runner = GeoKernelRunner(executable)
    command = runner.command("convex_hull", tmp_path / "in.json", tmp_path / "out.json", trace=True, pretty=True)
    assert "--algorithm" in command
    assert "convex_hull" in command
    assert "--trace" in command
    assert "--pretty" in command
