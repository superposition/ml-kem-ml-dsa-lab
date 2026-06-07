#!/usr/bin/env bash
set -euo pipefail

crate_files="$(find rust -type f -name Cargo.toml -print)"

if [[ -n "$crate_files" ]]; then
  cat >&2 <<EOF
Rust crate gate is closed.

Do not add a Rust Cargo.toml until the crate-introduction gate is intentionally
opened through issue #15. Found:

$crate_files
EOF
  exit 1
fi

echo "Rust crate gate is closed and no Rust Cargo.toml files were found."

