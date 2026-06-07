#!/usr/bin/env python3
"""Validate vector provenance and placeholder-vector manifests."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
import tempfile
from pathlib import Path
from typing import Any


HEX_SHA256 = re.compile(r"^[0-9a-f]{64}$")
DATE = re.compile(r"^\d{4}-\d{2}-\d{2}$")
ML_KEM_PARAMETER_SETS = {"ML-KEM-512", "ML-KEM-768", "ML-KEM-1024"}
ML_DSA_PARAMETER_SETS = {"ML-DSA-44", "ML-DSA-65", "ML-DSA-87"}


class ManifestError(ValueError):
  pass


def load_json(path: Path) -> Any:
  try:
    with path.open("r", encoding="utf-8") as handle:
      return json.load(handle)
  except json.JSONDecodeError as error:
    raise ManifestError(f"{path}: malformed JSON: {error.msg}") from error


def require_object(value: Any, context: str) -> dict[str, Any]:
  if not isinstance(value, dict):
    raise ManifestError(f"{context}: expected object")
  return value


def require_array(value: Any, context: str) -> list[Any]:
  if not isinstance(value, list):
    raise ManifestError(f"{context}: expected array")
  return value


def require_string(value: Any, context: str) -> str:
  if not isinstance(value, str) or not value:
    raise ManifestError(f"{context}: expected non-empty string")
  return value


def require_int(value: Any, context: str) -> int:
  if not isinstance(value, int):
    raise ManifestError(f"{context}: expected integer")
  return value


def require_bool(value: Any, context: str) -> bool:
  if not isinstance(value, bool):
    raise ManifestError(f"{context}: expected boolean")
  return value


def require_date(value: Any, context: str) -> str:
  text = require_string(value, context)
  if DATE.fullmatch(text) is None:
    raise ManifestError(f"{context}: expected YYYY-MM-DD date")
  return text


def require_sha256(value: Any, context: str) -> str:
  text = require_string(value, context)
  if HEX_SHA256.fullmatch(text) is None:
    raise ManifestError(f"{context}: expected lowercase SHA-256 hex digest")
  return text


def sha256_file(path: Path) -> str:
  digest = hashlib.sha256()
  with path.open("rb") as handle:
    for chunk in iter(lambda: handle.read(65536), b""):
      digest.update(chunk)
  return digest.hexdigest()


def allowed_parameter_sets(scheme: str) -> set[str]:
  if scheme == "ML-KEM":
    return ML_KEM_PARAMETER_SETS
  if scheme == "ML-DSA":
    return ML_DSA_PARAMETER_SETS
  if scheme == "mixed":
    return ML_KEM_PARAMETER_SETS | ML_DSA_PARAMETER_SETS
  raise ManifestError(f"unknown vector scheme: {scheme}")


def validate_parameter_sets(raw_sets: Any, scheme: str, context: str) -> list[str]:
  parameter_sets = require_array(raw_sets, context)
  allowed = allowed_parameter_sets(scheme)
  names: list[str] = []
  for index, value in enumerate(parameter_sets):
    name = require_string(value, f"{context}[{index}]")
    if name not in allowed:
      raise ManifestError(f"{context}[{index}]: unknown parameter set {name}")
    names.append(name)
  if not names:
    raise ManifestError(f"{context}: must list at least one parameter set")
  return names


def validate_placeholder_file(path: Path, repo_root: Path) -> None:
  data = require_object(load_json(path), str(path))
  cases = require_array(data.get("cases"), f"{path}: cases")
  for index, item in enumerate(cases):
    case = require_object(item, f"{path}: cases[{index}]")
    scheme = require_string(case.get("scheme"), f"{path}: cases[{index}].scheme")
    parameter_set = require_string(
        case.get("parameter_set"), f"{path}: cases[{index}].parameter_set")
    if parameter_set not in allowed_parameter_sets(scheme):
      raise ManifestError(
          f"{path}: cases[{index}].parameter_set: unknown parameter set {parameter_set}")
    require_string(case.get("id"), f"{path}: cases[{index}].id")
    require_string(case.get("operation"), f"{path}: cases[{index}].operation")
    expected = require_string(case.get("expected"), f"{path}: cases[{index}].expected")
    if expected != "not_implemented":
      raise ManifestError(f"{path}: cases[{index}].expected: unsupported expectation")
  if not path.is_relative_to(repo_root):
    raise ManifestError(f"{path}: placeholder file must live inside repository root")


def vector_set_digest(files: list[tuple[str, str, str]]) -> str:
  digest = hashlib.sha256()
  for role, file_name, file_hash in files:
    digest.update(f"{role}\0{file_name}\0{file_hash}\n".encode("utf-8"))
  return digest.hexdigest()


def validate_acvp_json_file(
    path: Path,
    *,
    scheme: str,
    mode: str,
    revision: str,
    parameter_sets: list[str],
    expected_sample: bool,
    context: str,
    require_group_parameter_sets: bool) -> tuple[int, int]:
  data = require_object(load_json(path), str(path))
  algorithm = require_string(data.get("algorithm"), f"{context}.algorithm")
  if algorithm != scheme:
    raise ManifestError(f"{context}.algorithm: expected {scheme}, got {algorithm}")
  file_mode = require_string(data.get("mode"), f"{context}.mode")
  if file_mode != mode:
    raise ManifestError(f"{context}.mode: expected {mode}, got {file_mode}")
  file_revision = require_string(data.get("revision"), f"{context}.revision")
  if file_revision != revision:
    raise ManifestError(f"{context}.revision: expected {revision}, got {file_revision}")
  is_sample = require_bool(data.get("isSample"), f"{context}.isSample")
  if is_sample != expected_sample:
    raise ManifestError(f"{context}.isSample: expected {expected_sample}, got {is_sample}")

  groups = require_array(data.get("testGroups"), f"{context}.testGroups")
  if not groups:
    raise ManifestError(f"{context}.testGroups: must contain at least one group")

  allowed = set(parameter_sets)
  seen_parameters: set[str] = set()
  case_count = 0
  for group_index, raw_group in enumerate(groups):
    group = require_object(raw_group, f"{context}.testGroups[{group_index}]")
    require_int(group.get("tgId"), f"{context}.testGroups[{group_index}].tgId")
    if require_group_parameter_sets:
      parameter_set = require_string(
          group.get("parameterSet"), f"{context}.testGroups[{group_index}].parameterSet")
      if parameter_set not in allowed:
        raise ManifestError(
            f"{context}.testGroups[{group_index}].parameterSet: unexpected {parameter_set}")
      seen_parameters.add(parameter_set)
    tests = require_array(group.get("tests"), f"{context}.testGroups[{group_index}].tests")
    if not tests:
      raise ManifestError(f"{context}.testGroups[{group_index}].tests: must not be empty")
    for test_index, raw_test in enumerate(tests):
      test = require_object(raw_test, f"{context}.testGroups[{group_index}].tests[{test_index}]")
      require_int(
          test.get("tcId"), f"{context}.testGroups[{group_index}].tests[{test_index}].tcId")
    case_count += len(tests)

  if require_group_parameter_sets and seen_parameters != allowed:
    missing = ", ".join(sorted(allowed - seen_parameters))
    raise ManifestError(f"{context}: missing parameter-set groups: {missing}")
  return len(groups), case_count


def validate_official_vector_set(
    vector_set: dict[str, Any],
    repo_root: Path,
    *,
    context: str,
    scheme: str,
    mode: str,
    revision: str,
    parameter_sets: list[str]) -> None:
  expected_group_count = require_int(vector_set.get("group_count"), f"{context}.group_count")
  expected_case_count = require_int(vector_set.get("case_count"), f"{context}.case_count")
  expected_sample = require_bool(vector_set.get("is_sample"), f"{context}.is_sample")
  files = require_array(vector_set.get("files"), f"{context}.files")
  if len(files) != 2:
    raise ManifestError(f"{context}.files: expected prompt and expectedResults files")

  seen_roles: set[str] = set()
  digest_entries: list[tuple[str, str, str]] = []
  counts_by_role: dict[str, tuple[int, int]] = {}
  for file_index, raw_file in enumerate(files):
    file_entry = require_object(raw_file, f"{context}.files[{file_index}]")
    role = require_string(file_entry.get("role"), f"{context}.files[{file_index}].role")
    if role not in {"prompt", "expectedResults"}:
      raise ManifestError(f"{context}.files[{file_index}].role: unsupported role {role}")
    if role in seen_roles:
      raise ManifestError(f"{context}.files[{file_index}].role: duplicate role {role}")
    seen_roles.add(role)
    require_string(file_entry.get("source_path"), f"{context}.files[{file_index}].source_path")
    file_name = require_string(file_entry.get("file"), f"{context}.files[{file_index}].file")
    expected_hash = require_sha256(
        file_entry.get("sha256"), f"{context}.files[{file_index}].sha256")
    vector_path = (repo_root / file_name).resolve()
    if not vector_path.exists():
      raise ManifestError(f"{file_name}: official vector file is missing")
    if not vector_path.is_relative_to(repo_root):
      raise ManifestError(f"{file_name}: official vector file must live inside repository root")
    actual_hash = sha256_file(vector_path)
    if actual_hash != expected_hash:
      raise ManifestError(f"{file_name}: SHA-256 mismatch, expected {expected_hash}, got {actual_hash}")
    digest_entries.append((role, file_name, expected_hash))
    counts_by_role[role] = validate_acvp_json_file(
        vector_path,
        scheme=scheme,
        mode=mode,
        revision=revision,
        parameter_sets=parameter_sets,
        expected_sample=expected_sample,
        context=f"{context}.files[{file_index}]",
        require_group_parameter_sets=role == "prompt")

  if seen_roles != {"prompt", "expectedResults"}:
    raise ManifestError(f"{context}.files: expected prompt and expectedResults roles")
  prompt_counts = counts_by_role["prompt"]
  expected_counts = counts_by_role["expectedResults"]
  if prompt_counts != expected_counts:
    raise ManifestError(
        f"{context}.files: prompt counts {prompt_counts} do not match expectedResults {expected_counts}")
  if prompt_counts != (expected_group_count, expected_case_count):
    raise ManifestError(
        f"{context}: expected {expected_group_count} groups and {expected_case_count} cases, got {prompt_counts}")

  expected_vector_hash = require_sha256(vector_set.get("sha256"), f"{context}.sha256")
  actual_vector_hash = vector_set_digest(digest_entries)
  if actual_vector_hash != expected_vector_hash:
    raise ManifestError(
        f"{context}.sha256: vector-set digest mismatch, expected {expected_vector_hash}, got {actual_vector_hash}")


def validate_manifest(path: Path, require_official: bool = False) -> list[str]:
  repo_root = path.parent.parent if path.parent.name == "test-vectors" else Path.cwd()
  manifest = require_object(load_json(path), str(path))
  if manifest.get("schema_version") != 1:
    raise ManifestError("schema_version: expected 1")
  require_date(manifest.get("retrieved_on"), "retrieved_on")

  sources = require_array(manifest.get("sources"), "sources")
  source_by_id: dict[str, dict[str, Any]] = {}
  for index, item in enumerate(sources):
    source = require_object(item, f"sources[{index}]")
    source_id = require_string(source.get("id"), f"sources[{index}].id")
    require_string(source.get("url"), f"sources[{index}].url")
    require_date(source.get("retrieved_on"), f"sources[{index}].retrieved_on")
    require_sha256(source.get("sha256"), f"sources[{index}].sha256")
    require_string(source.get("hash_target"), f"sources[{index}].hash_target")
    source_by_id[source_id] = source

  vector_sets = require_array(manifest.get("vector_sets"), "vector_sets")
  pending: list[str] = []
  official_ready = 0
  for index, item in enumerate(vector_sets):
    vector_set = require_object(item, f"vector_sets[{index}]")
    vector_id = require_string(vector_set.get("id"), f"vector_sets[{index}].id")
    kind = require_string(vector_set.get("kind"), f"vector_sets[{index}].kind")
    scheme = require_string(vector_set.get("scheme"), f"vector_sets[{index}].scheme")
    status = require_string(vector_set.get("status"), f"vector_sets[{index}].status")
    require_string(vector_set.get("source_url"), f"vector_sets[{index}].source_url")
    require_date(vector_set.get("retrieved_on"), f"vector_sets[{index}].retrieved_on")
    require_sha256(vector_set.get("sha256"), f"vector_sets[{index}].sha256")
    require_string(vector_set.get("hash_target"), f"vector_sets[{index}].hash_target")
    require_string(vector_set.get("mode"), f"vector_sets[{index}].mode")
    require_string(vector_set.get("revision"), f"vector_sets[{index}].revision")
    parameter_sets = validate_parameter_sets(
        vector_set.get("parameter_sets"), scheme, f"vector_sets[{index}].parameter_sets")

    if kind == "official-acvp":
      source_id = require_string(vector_set.get("source_id"), f"vector_sets[{index}].source_id")
      if source_id not in source_by_id:
        raise ManifestError(f"vector_sets[{index}].source_id: unknown source id {source_id}")
      if status == "pending":
        reason = require_string(
            vector_set.get("pending_reason"), f"vector_sets[{index}].pending_reason")
        pending.append(f"{vector_id}: {reason}")
      elif status == "vendored":
        validate_official_vector_set(
            vector_set,
            repo_root.resolve(),
            context=f"vector_sets[{index}]",
            scheme=scheme,
            mode=vector_set["mode"],
            revision=vector_set["revision"],
            parameter_sets=parameter_sets)
        official_ready += 1
      else:
        raise ManifestError(f"vector_sets[{index}].status: unsupported official status")
    elif kind == "repository-placeholder":
      if status != "placeholder":
        raise ManifestError(f"vector_sets[{index}].status: placeholder expected")
      file_name = require_string(vector_set.get("file"), f"vector_sets[{index}].file")
      fixture_path = (repo_root / file_name).resolve()
      if not fixture_path.exists():
        raise ManifestError(f"{file_name}: placeholder file is missing")
      expected = require_sha256(vector_set.get("sha256"), f"vector_sets[{index}].sha256")
      actual = sha256_file(fixture_path)
      if actual != expected:
        raise ManifestError(f"{file_name}: SHA-256 mismatch, expected {expected}, got {actual}")
      validate_placeholder_file(fixture_path, repo_root.resolve())
    else:
      raise ManifestError(f"vector_sets[{index}].kind: unsupported kind {kind}")

  if require_official and official_ready == 0:
    pending_text = "; ".join(pending) if pending else "no official vector entries found"
    raise ManifestError(f"official vector sets are not vendored: {pending_text}")

  return pending


def run_self_test() -> None:
  with tempfile.TemporaryDirectory() as tmp:
    bad_json = Path(tmp) / "bad.json"
    bad_json.write_text("{not-json", encoding="utf-8")
    try:
      load_json(bad_json)
    except ManifestError as error:
      if "malformed JSON" not in str(error):
        raise
    else:
      raise ManifestError("self-test expected malformed JSON rejection")

    pending_manifest = Path(tmp) / "manifest.json"
    pending_manifest.write_text(
        json.dumps({
            "schema_version": 1,
            "retrieved_on": "2026-06-07",
            "sources": [{
                "id": "source",
                "url": "https://example.invalid/source",
                "retrieved_on": "2026-06-07",
                "sha256": "0" * 64,
                "hash_target": "self-test source"
            }],
            "vector_sets": [{
                "id": "pending-official",
                "kind": "official-acvp",
                "source_id": "source",
                "source_url": "https://example.invalid/source",
                "retrieved_on": "2026-06-07",
                "sha256": "0" * 64,
                "hash_target": "self-test source",
                "standard": "FIPS 203",
                "scheme": "ML-KEM",
                "mode": "keyGen",
                "revision": "FIPS203",
                "parameter_sets": ["ML-KEM-512"],
                "status": "pending",
                "pending_reason": "self-test official vector is intentionally absent"
            }]
        }),
        encoding="utf-8")
    try:
      validate_manifest(pending_manifest, require_official=True)
    except ManifestError as error:
      if "official vector sets are not vendored" not in str(error):
        raise
    else:
      raise ManifestError("self-test expected strict official-vector gate rejection")

    repo_root = Path(tmp) / "repo"
    vector_dir = repo_root / "test-vectors" / "acvp" / "self-test"
    vector_dir.mkdir(parents=True)
    prompt = {
        "vsId": 1,
        "algorithm": "ML-KEM",
        "mode": "keyGen",
        "revision": "FIPS203",
        "isSample": False,
        "testGroups": [{
            "tgId": 1,
            "testType": "AFT",
            "parameterSet": "ML-KEM-512",
            "tests": [{"tcId": 1, "d": "00", "z": "00"}]
        }]
    }
    expected = {
        "vsId": 1,
        "algorithm": "ML-KEM",
        "mode": "keyGen",
        "revision": "FIPS203",
        "isSample": False,
        "testGroups": [{
            "tgId": 1,
            "tests": [{"tcId": 1, "ek": "00", "dk": "00"}]
        }]
    }
    prompt_path = vector_dir / "prompt.json"
    expected_path = vector_dir / "expectedResults.json"
    prompt_path.write_text(json.dumps(prompt), encoding="utf-8")
    expected_path.write_text(json.dumps(expected), encoding="utf-8")
    prompt_file = "test-vectors/acvp/self-test/prompt.json"
    expected_file = "test-vectors/acvp/self-test/expectedResults.json"
    prompt_hash = sha256_file(prompt_path)
    expected_hash = sha256_file(expected_path)
    official_manifest = repo_root / "test-vectors" / "manifest.json"
    official_manifest.write_text(
        json.dumps({
            "schema_version": 1,
            "retrieved_on": "2026-06-07",
            "sources": [{
                "id": "source",
                "url": "https://example.invalid/source",
                "retrieved_on": "2026-06-07",
                "sha256": "0" * 64,
                "hash_target": "self-test source"
            }],
            "vector_sets": [{
                "id": "vendored-official",
                "kind": "official-acvp",
                "source_id": "source",
                "source_url": "https://example.invalid/source",
                "retrieved_on": "2026-06-07",
                "sha256": vector_set_digest([
                    ("prompt", prompt_file, prompt_hash),
                    ("expectedResults", expected_file, expected_hash)
                ]),
                "hash_target": "self-test vector-set digest",
                "standard": "FIPS 203",
                "scheme": "ML-KEM",
                "mode": "keyGen",
                "revision": "FIPS203",
                "parameter_sets": ["ML-KEM-512"],
                "status": "vendored",
                "is_sample": False,
                "group_count": 1,
                "case_count": 1,
                "files": [
                    {
                        "role": "prompt",
                        "file": prompt_file,
                        "source_path": "self-test/prompt.json",
                        "sha256": prompt_hash
                    },
                    {
                        "role": "expectedResults",
                        "file": expected_file,
                        "source_path": "self-test/expectedResults.json",
                        "sha256": expected_hash
                    }
                ]
            }]
        }),
        encoding="utf-8")
    validate_manifest(official_manifest, require_official=True)


def main() -> int:
  parser = argparse.ArgumentParser()
  parser.add_argument(
      "--manifest",
      type=Path,
      default=Path("test-vectors/manifest.json"),
      help="Path to vector manifest JSON.")
  parser.add_argument(
      "--require-official",
      action="store_true",
      help="Fail unless at least one official vector set is vendored.")
  parser.add_argument(
      "--self-test",
      action="store_true",
      help="Run parser self-tests before validating the manifest.")
  args = parser.parse_args()

  try:
    if args.self_test:
      run_self_test()
    pending = validate_manifest(args.manifest, require_official=args.require_official)
  except ManifestError as error:
    print(f"Vector manifest validation failed: {error}", file=sys.stderr)
    return 1

  if pending:
    print("Official vector sets pending:")
    for item in pending:
      print(f"- {item}")
  print(f"Vector manifest validated: {args.manifest}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
