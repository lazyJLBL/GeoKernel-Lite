from __future__ import annotations

from dataclasses import dataclass
import json
from pathlib import Path
from typing import Any, Iterable


PROJECT_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_CATALOG = PROJECT_ROOT / "tests" / "cases" / "catalog.json"


@dataclass(frozen=True)
class CaseRecord:
    name: str
    algorithm: str
    description: str
    input: dict[str, Any]
    expected: dict[str, Any]
    tags: tuple[str, ...]


def _case_from_dict(data: dict[str, Any]) -> CaseRecord:
    return CaseRecord(
        name=data["name"],
        algorithm=data["algorithm"],
        description=data.get("description", ""),
        input=data.get("input", {}),
        expected=data.get("expected", {}),
        tags=tuple(data.get("tags", [])),
    )


def load_case(path: str | Path) -> dict[str, Any]:
    with Path(path).open("r", encoding="utf-8") as handle:
        return json.load(handle)


def _records_from_path(path: str | Path) -> list[CaseRecord]:
    data = load_case(path)
    cases = data.get("cases", data if isinstance(data, list) else [])
    return [_case_from_dict(case) for case in cases]


def load_case_catalog(path: str | Path = DEFAULT_CATALOG) -> list[CaseRecord]:
    path = Path(path)
    if path != DEFAULT_CATALOG:
        return _records_from_path(path)

    records: list[CaseRecord] = []
    for case_file in sorted(DEFAULT_CATALOG.parent.glob("*.json")):
        records.extend(_records_from_path(case_file))
    return records


def list_cases(algorithm: str | None = None, tags: Iterable[str] | None = None) -> list[CaseRecord]:
    selected_tags = set(tags or [])
    cases = load_case_catalog()
    if algorithm:
        cases = [case for case in cases if case.algorithm == algorithm]
    if selected_tags:
        cases = [case for case in cases if selected_tags.issubset(set(case.tags))]
    return cases
