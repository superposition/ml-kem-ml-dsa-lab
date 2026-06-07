#!/usr/bin/env bash
set -euo pipefail

crate_files="$(find rust -type f -name Cargo.toml -print)"
expected_crate="rust/pqcore/Cargo.toml"

if [[ "$crate_files" != "$expected_crate" ]]; then
  cat >&2 <<EOF
Rust crate gate is open only for the intentional issue #15 crate.

Expected exactly:

$expected_crate

Found:

${crate_files:-<none>}
EOF
  exit 1
fi

if ! grep -q "publish = false" "$expected_crate"; then
  echo "Rust crate must remain publish = false until a release policy exists." >&2
  exit 1
fi

if ! grep -qi "not production cryptography" rust/pqcore/README.md; then
  echo "Rust crate README must state that the crate is not production cryptography." >&2
  exit 1
fi

if grep -R -nE "^[[:space:]]*pub[[:space:]]+(mod|fn)[[:space:]]+ml_(kem|dsa)" rust/pqcore/src; then
  echo "Rust crate must not expose public ML-KEM or ML-DSA API stubs yet." >&2
  exit 1
fi

echo "Rust crate gate is open for issue #15 and the internal crate shape is validated."
