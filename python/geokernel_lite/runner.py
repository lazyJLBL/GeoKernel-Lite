from __future__ import annotations

import json
from pathlib import Path
import subprocess
import tempfile
from typing import Any


PROJECT_ROOT = Path(__file__).resolve().parents[2]


def locate_demo_executable(root: Path = PROJECT_ROOT) -> Path | None:
    candidates = [
        root / "build" / "geokernel_demo.exe",
        root / "build" / "Debug" / "geokernel_demo.exe",
        root / "build" / "Release" / "geokernel_demo.exe",
        root / "build" / "apps" / "geokernel_demo.exe",
        root / "build" / "geokernel_demo",
        root / "build" / "Debug" / "geokernel_demo",
        root / "build" / "Release" / "geokernel_demo",
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return None


class GeoKernelRunner:
    def __init__(self, executable: str | Path | None = None) -> None:
        resolved = Path(executable) if executable else locate_demo_executable()
        if resolved is None:
            raise FileNotFoundError("geokernel_demo executable was not found under build/")
        self.executable = resolved

    def command(self, algorithm: str, input_path: Path, output_path: Path, trace: bool = True, pretty: bool = True) -> list[str]:
        command = [
            str(self.executable),
            "--algorithm",
            algorithm,
            "--input",
            str(input_path),
            "--output",
            str(output_path),
        ]
        if trace:
            command.append("--trace")
        if pretty:
            command.append("--pretty")
        return command

    def run(self, algorithm: str, payload: dict[str, Any], trace: bool = True) -> dict[str, Any]:
        with tempfile.TemporaryDirectory(prefix="geokernel_") as temp_dir:
            temp_path = Path(temp_dir)
            input_path = temp_path / "input.json"
            output_path = temp_path / "output.json"
            input_path.write_text(json.dumps({"input": payload}, indent=2), encoding="utf-8")
            completed = subprocess.run(
                self.command(algorithm, input_path, output_path, trace=trace),
                cwd=PROJECT_ROOT,
                text=True,
                capture_output=True,
                check=False,
            )
            if completed.returncode != 0:
                raise RuntimeError(completed.stderr.strip() or completed.stdout.strip())
            return json.loads(output_path.read_text(encoding="utf-8"))
