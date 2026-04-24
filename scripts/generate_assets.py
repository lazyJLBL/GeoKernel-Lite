from __future__ import annotations

import json
from pathlib import Path
import sys
from typing import Any

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "python"))

from geokernel_lite.visualization import figure_for_result


ASSETS = ROOT / "assets"
EXAMPLES = ROOT / "examples"


OUTPUTS: dict[str, dict[str, Any]] = {
    "convex_hull": {
        "result": {
            "hull": [[0, 0], [1, 0], [1, 1], [0, 1]],
            "area": 1.0,
            "perimeter": 4.0,
        },
        "trace": [],
    },
    "segment_intersection": {
        "result": {
            "has_intersection": True,
            "pairs": [
                {
                    "first": 0,
                    "second": 1,
                    "intersection": {"type": "point", "point": [2, 2]},
                }
            ],
        },
        "trace": [],
    },
    "polygon_clipping": {
        "result": {
            "status": "polygon",
            "polygon": [[0, 0], [2, 0], [2, 2], [0, 2]],
            "area": 4.0,
        },
        "trace": [],
    },
    "half_plane_intersection": {
        "result": {
            "status": "bounded_polygon",
            "polygon": [[0, 0], [2, 0], [2, 2], [0, 2]],
            "area": 4.0,
            "clipped_by_bounding_box": True,
        },
        "trace": [],
    },
    "closest_pair": {
        "result": {
            "valid": True,
            "p1": [1, 1],
            "p2": [1.1, 1.05],
            "distance": 0.1118,
        },
        "trace": [],
    },
    "triangulation": {
        "result": {
            "valid": True,
            "triangles": [
                [[0, 3], [0, 0], [4, 0]],
                [[0, 3], [4, 0], [4, 1]],
                [[0, 3], [4, 1], [2, 1]],
                [[2, 1], [2, 3], [0, 3]],
            ],
            "polygon_area": 8.0,
            "triangles_area": 8.0,
            "area_error": 0.0,
        },
        "trace": [],
    },
}


FILENAMES = {
    "convex_hull": "convex_hull_demo.png",
    "segment_intersection": "segment_intersection_demo.png",
    "polygon_clipping": "polygon_clipping_demo.png",
    "half_plane_intersection": "half_plane_intersection_demo.png",
    "closest_pair": "closest_pair_demo.png",
    "triangulation": "triangulation_demo.png",
}


COLORS = {
    "bg": (248, 250, 252),
    "grid": (226, 232, 240),
    "axis": (148, 163, 184),
    "point": (15, 107, 120),
    "result": (196, 59, 59),
    "accent": (47, 133, 90),
    "orange": (217, 119, 6),
    "text": (15, 23, 42),
}


def load_example(name: str) -> dict[str, Any]:
    with (EXAMPLES / f"{name}.json").open("r", encoding="utf-8") as handle:
        return json.load(handle)


def collect_points(payload: dict[str, Any], output: dict[str, Any]) -> list[list[float]]:
    points: list[list[float]] = []
    input_data = payload.get("input", payload)
    for key in ("points", "subject", "clipper", "polygon"):
        points.extend(input_data.get(key, []))
    for segment in input_data.get("segments", []):
        points.extend(segment)
    for hp in input_data.get("halfplanes", []):
        p = hp["p"]
        d = hp["dir"]
        points.extend([p, [p[0] + d[0], p[1] + d[1]]])

    result = output.get("result", {})
    for key in ("hull", "polygon"):
        points.extend(result.get(key, []))
    if "diameter" in result:
        points.extend([result["diameter"]["p1"], result["diameter"]["p2"]])
    if "p1" in result and "p2" in result:
        points.extend([result["p1"], result["p2"]])
    for triangle in result.get("triangles", []):
        points.extend(triangle)
    return points or [[0, 0], [1, 1]]


def transform(points: list[list[float]], width: int, height: int):
    xs = [p[0] for p in points]
    ys = [p[1] for p in points]
    min_x, max_x = min(xs), max(xs)
    min_y, max_y = min(ys), max(ys)
    if min_x == max_x:
        min_x -= 1
        max_x += 1
    if min_y == max_y:
        min_y -= 1
        max_y += 1
    pad = 72
    scale = min((width - 2 * pad) / (max_x - min_x), (height - 2 * pad) / (max_y - min_y))

    def convert(p: list[float]) -> tuple[int, int]:
        x = int(pad + (p[0] - min_x) * scale)
        y = int(height - pad - (p[1] - min_y) * scale)
        return x, y

    return convert


def draw_poly(draw: ImageDraw.ImageDraw, convert, points: list[list[float]], color, width: int = 4, close: bool = True) -> None:
    if len(points) < 2:
        return
    converted = [convert(p) for p in points]
    if close:
        converted.append(converted[0])
    draw.line(converted, fill=color, width=width, joint="curve")


def draw_segments(draw: ImageDraw.ImageDraw, convert, segments: list[list[list[float]]], color, width: int = 4) -> None:
    for segment in segments:
        draw.line([convert(segment[0]), convert(segment[1])], fill=color, width=width)


def draw_points(draw: ImageDraw.ImageDraw, convert, points: list[list[float]], color, radius: int = 6) -> None:
    for point in points:
        x, y = convert(point)
        draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill=color, outline=(255, 255, 255), width=2)


def write_fallback_png(name: str, payload: dict[str, Any], output: dict[str, Any], path: Path) -> None:
    width, height = 1200, 760
    image = Image.new("RGB", (width, height), COLORS["bg"])
    draw = ImageDraw.Draw(image)
    font = ImageFont.load_default()
    convert = transform(collect_points(payload, output), width, height)

    for x in range(72, width - 72, 72):
        draw.line((x, 72, x, height - 72), fill=COLORS["grid"])
    for y in range(72, height - 72, 72):
        draw.line((72, y, width - 72, y), fill=COLORS["grid"])

    input_data = payload.get("input", payload)
    result = output.get("result", {})
    draw.text((36, 28), name.replace("_", " ").title(), fill=COLORS["text"], font=font)

    if "segments" in input_data:
        draw_segments(draw, convert, input_data["segments"], COLORS["axis"], 4)
    if "subject" in input_data:
        draw_poly(draw, convert, input_data["subject"], COLORS["point"], 3)
    if "clipper" in input_data:
        draw_poly(draw, convert, input_data["clipper"], COLORS["result"], 3)
    if "polygon" in input_data:
        draw_poly(draw, convert, input_data["polygon"], COLORS["point"], 3)
    if "points" in input_data:
        draw_points(draw, convert, input_data["points"], COLORS["point"], 7)

    if result.get("hull"):
        draw_poly(draw, convert, result["hull"], COLORS["result"], 5)
    if result.get("polygon"):
        draw_poly(draw, convert, result["polygon"], COLORS["accent"], 5)
    if result.get("p1") and result.get("p2"):
        draw_segments(draw, convert, [[result["p1"], result["p2"]]], COLORS["result"], 5)
        draw_points(draw, convert, [result["p1"], result["p2"]], COLORS["result"], 8)
    for triangle in result.get("triangles", []):
        draw_poly(draw, convert, triangle, COLORS["accent"], 3)

    image.save(path)


def write_asset(name: str) -> None:
    payload = load_example(name)
    output = OUTPUTS[name]
    fig = figure_for_result(name, payload, output)
    path = ASSETS / FILENAMES[name]
    try:
        fig.write_image(str(path), width=1200, height=760, scale=2)
    except Exception:
        write_fallback_png(name, payload, output, path)


def main() -> None:
    ASSETS.mkdir(parents=True, exist_ok=True)
    for name in FILENAMES:
        write_asset(name)
        print(f"wrote {ASSETS / FILENAMES[name]}")


if __name__ == "__main__":
    main()
