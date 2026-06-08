# Rust Derivation: Official Vector Ingestion and Differential Testing

The Rust vector runner derives its inputs from `docs/vector-ingestion.md` and
`test-vectors/manifest.json`.

It is not a conformance pass yet. Rust public ML-KEM and ML-DSA algorithms are unavailable, so
official ACVP cases are reported as pending with location metadata.

## Current Type Shape

The crate exposes internal vector plumbing along these lines:

```text
VectorStatus::{Passed, Pending, Skipped, Failed}
VectorCaseLocation
VectorCaseReport
VectorSummary
VectorRunReport
run_manifest(path)
```

The manifest format must stay language-neutral so C++ and Rust consume the same provenance and vector
metadata.

## Implemented Red Tests

1. Manifest parsing rejects malformed JSON.
2. Manifest source ids are checked for presence and uniqueness.
3. ML-KEM parameter-set names map to Rust-known names.
4. ML-DSA parameter-set names map to Rust-known names.
5. Vendored official vectors produce `Pending` while Rust algorithms are unavailable.
6. Placeholder repository fixtures produce `Pending` for expected `not_implemented` behavior.
7. Failure reports include scheme, mode, parameter set, vector-set id, group id, and test-case id.

## Future Differential Tests

1. Rust outputs match official expected outputs.
2. C++ outputs match official expected outputs.
3. C++ and Rust outputs match each other.
4. Secret-bearing vector fields remain absent from runner logs.

## Readiness Caveat

Differential agreement does not prove cryptographic security. It catches implementation drift after the
standard-conformance vector path is in place.
