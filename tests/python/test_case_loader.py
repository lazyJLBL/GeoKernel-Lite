from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "python"))

from geokernel_lite.case_loader import load_case_catalog, list_cases


def test_catalog_contains_at_least_50_boundary_cases():
    cases = load_case_catalog()
    assert len(cases) >= 50
    assert {case.algorithm for case in cases} >= {
        "convex_hull",
        "segment_intersection",
        "polygon_clipping",
        "half_plane_intersection",
        "closest_pair",
        "triangulation",
        "delaunay",
        "predicate_compare",
    }


def test_cases_have_required_fields():
    for case in load_case_catalog():
        assert case.name
        assert case.algorithm
        assert isinstance(case.input, dict)
        assert isinstance(case.expected, dict)


def test_list_cases_filters_algorithm():
    segment_cases = list_cases("segment_intersection")
    assert segment_cases
    assert all(case.algorithm == "segment_intersection" for case in segment_cases)
