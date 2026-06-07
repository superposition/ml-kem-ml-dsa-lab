#!/usr/bin/env python3
"""Validate the constant-time and side-channel review gate manifest."""

from __future__ import annotations

import argparse
import json
import sys
import tempfile
from pathlib import Path
from typing import Any


REQUIRED_MODULE_IDS = {
  "field-polynomial-ntt",
  "encoding-compression",
  "sampling",
  "ml-kem-pke",
  "ml-kem-kem",
  "ml-dsa-helpers",
  "ml-dsa-signature",
  "public-api-stubs",
}


class SideChannelManifestError(ValueError):
  pass


def load_json(path: Path) -> Any:
  try:
    with path.open("r", encoding="utf-8") as handle:
      return json.load(handle)
  except json.JSONDecodeError as error:
    raise SideChannelManifestError(f"{path}: malformed JSON: {error.msg}") from error


def require_object(value: Any, context: str) -> dict[str, Any]:
  if not isinstance(value, dict):
    raise SideChannelManifestError(f"{context}: expected object")
  return value


def require_array(value: Any, context: str) -> list[Any]:
  if not isinstance(value, list):
    raise SideChannelManifestError(f"{context}: expected array")
  if not value:
    raise SideChannelManifestError(f"{context}: expected non-empty array")
  return value


def require_string(value: Any, context: str) -> str:
  if not isinstance(value, str) or not value:
    raise SideChannelManifestError(f"{context}: expected non-empty string")
  return value


def require_bool(value: Any, context: str) -> bool:
  if not isinstance(value, bool):
    raise SideChannelManifestError(f"{context}: expected boolean")
  return value


def validate_note_array(value: Any, context: str) -> None:
  items = require_array(value, context)
  for index, item in enumerate(items):
    if isinstance(item, str):
      require_string(item, f"{context}[{index}]")
    else:
      entry = require_object(item, f"{context}[{index}]")
      require_string(entry.get("description"), f"{context}[{index}].description")
      require_string(entry.get("status"), f"{context}[{index}].status")


def validate_path(path_text: str, repo_root: Path, context: str) -> None:
  path = repo_root / path_text
  if not path.exists():
    raise SideChannelManifestError(f"{context}: referenced path does not exist: {path_text}")


def validate_path_array(value: Any, repo_root: Path, context: str) -> None:
  items = require_array(value, context)
  for index, item in enumerate(items):
    path_text = require_string(item, f"{context}[{index}]")
    validate_path(path_text, repo_root, f"{context}[{index}]")


def validate_module(module: dict[str, Any], context: str, repo_root: Path) -> str:
  module_id = require_string(module.get("id"), f"{context}.id")
  doc = require_string(module.get("doc"), f"{context}.doc")
  validate_path(doc, repo_root, f"{context}.doc")
  validate_path_array(module.get("source_files"), repo_root, f"{context}.source_files")
  require_string(module.get("review_status"), f"{context}.review_status")
  validate_note_array(module.get("public_inputs"), f"{context}.public_inputs")
  validate_note_array(module.get("secret_inputs"), f"{context}.secret_inputs")
  validate_note_array(
      module.get("secret_derived_intermediates"), f"{context}.secret_derived_intermediates")
  validate_note_array(
      module.get("secret_dependent_branches"), f"{context}.secret_dependent_branches")
  validate_note_array(
      module.get("secret_dependent_memory_access"),
      f"{context}.secret_dependent_memory_access")
  validate_note_array(module.get("error_paths"), f"{context}.error_paths")
  require_string(module.get("side_channel_note"), f"{context}.side_channel_note")
  return module_id


def validate_required_review(
    review: dict[str, Any],
    context: str,
    module_ids: set[str],
    finding_ids: set[str]) -> None:
  module_id = require_string(review.get("module_id"), f"{context}.module_id")
  if module_id not in module_ids:
    raise SideChannelManifestError(f"{context}.module_id: unknown module id {module_id}")
  require_string(review.get("status"), f"{context}.status")
  finding_id = require_string(review.get("finding_id"), f"{context}.finding_id")
  if finding_id not in finding_ids:
    raise SideChannelManifestError(f"{context}.finding_id: unknown finding id {finding_id}")
  if not require_bool(review.get("production_blocker"), f"{context}.production_blocker"):
    raise SideChannelManifestError(f"{context}.production_blocker: expected true")
  require_string(review.get("note"), f"{context}.note")


def validate_manifest(path: Path, require_complete: bool = False) -> None:
  repo_root = path.resolve().parents[2] if path.parent.name == "side-channel" else Path.cwd()
  manifest = require_object(load_json(path), str(path))
  if manifest.get("schema_version") != 1:
    raise SideChannelManifestError("schema_version: expected 1")
  if manifest.get("gate") != "constant-time":
    raise SideChannelManifestError("gate: expected constant-time")
  gate_status = require_string(manifest.get("gate_status"), "gate_status")
  production_status = require_bool(manifest.get("production_status"), "production_status")
  if production_status and gate_status != "complete":
    raise SideChannelManifestError("production_status: expected false until gate is complete")
  if manifest.get("timing_tests") != "advisory-only":
    raise SideChannelManifestError("timing_tests: expected advisory-only")
  require_string(manifest.get("timing_tests_note"), "timing_tests_note")

  secret_debug_policy = require_object(
      manifest.get("secret_debug_policy"), "secret_debug_policy")
  if require_bool(
      secret_debug_policy.get("raw_debug_output_allowed"),
      "secret_debug_policy.raw_debug_output_allowed"):
    raise SideChannelManifestError(
        "secret_debug_policy.raw_debug_output_allowed: expected false")
  require_string(secret_debug_policy.get("cxx_annotation_type"),
                 "secret_debug_policy.cxx_annotation_type")
  require_string(secret_debug_policy.get("cxx_test"), "secret_debug_policy.cxx_test")
  require_string(secret_debug_policy.get("note"), "secret_debug_policy.note")

  modules = require_array(manifest.get("modules"), "modules")
  module_ids = {
      validate_module(
          require_object(module, f"modules[{index}]"), f"modules[{index}]", repo_root)
      for index, module in enumerate(modules)
  }
  missing_modules = REQUIRED_MODULE_IDS - module_ids
  if missing_modules:
    raise SideChannelManifestError(
        f"modules: missing required module ids {', '.join(sorted(missing_modules))}")

  findings = require_array(manifest.get("findings"), "findings")
  finding_ids: set[str] = set()
  production_blockers = 0
  for index, item in enumerate(findings):
    finding = require_object(item, f"findings[{index}]")
    finding_id = require_string(finding.get("id"), f"findings[{index}].id")
    module_id = require_string(finding.get("module_id"), f"findings[{index}].module_id")
    if module_id not in module_ids:
      raise SideChannelManifestError(
          f"findings[{index}].module_id: unknown module id {module_id}")
    require_string(finding.get("title"), f"findings[{index}].title")
    status = require_string(finding.get("status"), f"findings[{index}].status")
    if "closed" in status and gate_status != "complete":
      raise SideChannelManifestError(
          f"findings[{index}].status: closed finding before gate completion")
    if require_bool(finding.get("production_blocker"), f"findings[{index}].production_blocker"):
      production_blockers += 1
    require_string(finding.get("public_tracking"), f"findings[{index}].public_tracking")
    finding_ids.add(finding_id)

  if production_blockers == 0:
    raise SideChannelManifestError("findings: expected at least one production blocker")

  required_reviews = require_object(
      manifest.get("required_reviews"), "required_reviews")
  for key in ("ml_kem_decapsulation_failure_handling", "ml_dsa_signing_rejection_behavior"):
    validate_required_review(
        require_object(required_reviews.get(key), f"required_reviews.{key}"),
        f"required_reviews.{key}",
        module_ids,
        finding_ids)

  if require_complete and gate_status != "complete":
    raise SideChannelManifestError(
        f"constant-time gate is not complete: {gate_status}")


def run_self_test() -> None:
  with tempfile.TemporaryDirectory() as tmp:
    bad_json = Path(tmp) / "bad.json"
    bad_json.write_text("{not-json", encoding="utf-8")
    try:
      load_json(bad_json)
    except SideChannelManifestError as error:
      if "malformed JSON" not in str(error):
        raise
    else:
      raise SideChannelManifestError("self-test expected malformed JSON rejection")


def main() -> int:
  parser = argparse.ArgumentParser()
  parser.add_argument(
      "--manifest",
      type=Path,
      default=Path("audits/side-channel/manifest.json"),
      help="Path to the side-channel review manifest.")
  parser.add_argument(
      "--require-complete",
      action="store_true",
      help="Fail unless the constant-time gate is marked complete.")
  parser.add_argument(
      "--self-test",
      action="store_true",
      help="Run parser self-tests before validating the manifest.")
  args = parser.parse_args()

  try:
    if args.self_test:
      run_self_test()
    validate_manifest(args.manifest, require_complete=args.require_complete)
  except SideChannelManifestError as error:
    print(f"Side-channel manifest validation failed: {error}", file=sys.stderr)
    return 1

  print(f"Side-channel manifest validated: {args.manifest}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
