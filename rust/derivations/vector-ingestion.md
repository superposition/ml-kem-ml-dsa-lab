# Rust Derivation: Official Vector Ingestion and Differential Testing

There is intentionally no Rust crate for this work yet.

The future Rust vector runner should derive its inputs from `docs/vector-ingestion.md` and
`test-vectors/manifest.json`.

## Future Type Shape

The eventual Rust crate should expose test-only vector plumbing along these lines:

```text
VectorManifest
VectorSet
VectorCase
VectorOutcome::{Passed, Failed, Pending}
load_vector_manifest(path)
run_ml_kem_vector_case(case)
run_ml_dsa_vector_case(case)
compare_cpp_and_rust_outputs(case)
```

The manifest format must stay language-neutral so C++ and Rust consume the same provenance and vector
metadata.

## Future Red Tests

1. Manifest parsing rejects malformed JSON.
2. Manifest parsing rejects source entries without retrieval dates.
3. Manifest parsing rejects source entries without SHA-256 hashes.
4. ML-KEM parameter-set names map to Rust parameter enums.
5. ML-DSA parameter-set names map to Rust parameter enums.
6. Missing official vectors produce `Pending` with the manifest reason.
7. Placeholder repository fixtures are never marked as official.
8. C++ and Rust runners consume the same vector-set id and test-case id.
9. C++ and Rust outputs match official expected outputs.
10. C++ and Rust outputs match each other.
11. Failure reports include scheme, mode, parameter set, vector-set id, group id, and test-case id.
12. No Rust crate is introduced until issue #15 intentionally opens the crate gate.

## Readiness Caveat

Differential agreement does not prove cryptographic security. It catches implementation drift after the
standard-conformance vector path is in place.
