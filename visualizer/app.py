from __future__ import annotations

import json
from pathlib import Path
import sys
from typing import Any

import pandas as pd
import streamlit as st

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "python"))

from geokernel_lite.case_loader import list_cases
from geokernel_lite.runner import GeoKernelRunner, locate_demo_executable
from geokernel_lite.visualization import figure_for_result


ALGORITHMS = [
    "convex_hull",
    "rotating_calipers",
    "segment_intersection",
    "half_plane_intersection",
    "polygon_clipping",
    "closest_pair",
    "triangulation",
    "delaunay",
]


def default_payload(algorithm: str) -> dict[str, Any]:
    cases = list_cases(algorithm)
    if cases:
        return cases[0].input
    return {}


def render_algorithm_tab(algorithm: str) -> None:
    cases = list_cases(algorithm)
    names = [case.name for case in cases]
    selected_name = st.selectbox("Case", names, key=f"{algorithm}_case") if names else None
    selected = next((case for case in cases if case.name == selected_name), None)
    payload = selected.input if selected else default_payload(algorithm)
    source = st.text_area("Input JSON", json.dumps(payload, indent=2), height=260, key=f"{algorithm}_json")
    trace = st.toggle("Trace", value=True, key=f"{algorithm}_trace")
    try:
        parsed_payload = json.loads(source)
    except json.JSONDecodeError as exc:
        st.error(f"Invalid input JSON: {exc}")
        return

    executable = locate_demo_executable()
    output = None
    if st.button("Run", key=f"{algorithm}_run", type="primary"):
        try:
            output = GeoKernelRunner(executable).run(algorithm, parsed_payload, trace=trace)
            st.session_state[f"{algorithm}_output"] = output
        except Exception as exc:
            st.error(str(exc))
    output = output or st.session_state.get(f"{algorithm}_output")

    if executable is None:
        st.warning("Build `geokernel_demo` first to execute C++ algorithms from the visualizer.")

    step = None
    if output and output.get("trace"):
        step = st.slider("Trace step", 0, len(output["trace"]) - 1, len(output["trace"]) - 1, key=f"{algorithm}_step")

    fig = figure_for_result(algorithm, {"input": parsed_payload}, output, step)
    st.plotly_chart(fig, use_container_width=True)

    if output:
        left, right = st.columns(2)
        left.json(output.get("summary", {}))
        right.json(output.get("warnings", []))
        st.json(output.get("result", {}), expanded=False)
        if output.get("trace"):
            rows = [
                {
                    "step": item.get("step_index"),
                    "phase": item.get("phase"),
                    "message": item.get("message"),
                    **item.get("metrics", {}),
                }
                for item in output["trace"]
            ]
            st.dataframe(pd.DataFrame(rows), use_container_width=True)


def render_robustness_tab() -> None:
    cases = list_cases()
    st.metric("Boundary cases", len(cases))
    algorithms = sorted({case.algorithm for case in cases})
    selected = st.multiselect("Algorithms", algorithms, default=algorithms)
    rows = [
        {
            "name": case.name,
            "algorithm": case.algorithm,
            "description": case.description,
            "tags": ", ".join(case.tags),
        }
        for case in cases
        if case.algorithm in selected
    ]
    st.dataframe(pd.DataFrame(rows), use_container_width=True, height=560)


def main() -> None:
    st.set_page_config(page_title="GeoKernel-Lite", layout="wide")
    st.title("GeoKernel-Lite")
    tabs = st.tabs([name.replace("_", " ").title() for name in ALGORITHMS] + ["Robustness Cases"])
    for algorithm, tab in zip(ALGORITHMS, tabs):
        with tab:
            render_algorithm_tab(algorithm)
    with tabs[-1]:
        render_robustness_tab()


if __name__ == "__main__":
    main()
