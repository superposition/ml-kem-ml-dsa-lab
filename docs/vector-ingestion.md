# Official Vector Ingestion and Differential Testing

This document defines the vector-ingestion gate for ML-KEM and ML-DSA.

## Source Pointers

| Source | Role | Current repository status |
| --- | --- | --- |
| FIPS 203 final | ML-KEM algorithm source | Referenced for algorithm mapping |
| FIPS 204 final | ML-DSA algorithm source | Referenced for algorithm mapping |
| ACVP ML-KEM JSON Specification | ML-KEM vector-set schema planning | Source manifest recorded; official vectors pending |
| ACVP ML-DSA JSON Specification | ML-DSA vector-set schema planning | Source manifest recorded; official vectors pending |

The ACVP pages define JSON schemas and validation-session behavior. They are not small KAT bundles
that can be copied directly into this repository. Official vector sets should be acquired through the ACVP
workflow and then recorded with provenance before being used as conformance evidence.

## Manifest

The machine-readable manifest lives at:

```text
test-vectors/manifest.json
```

Every source entry records:

- source identifier,
- URL,
- retrieval date,
- SHA-256 hash,
- hash target.

Every vector-set entry records:

- source URL,
- retrieval date,
- SHA-256 hash,
- standard,
- scheme,
- mode,
- revision,
- parameter sets,
- status.

Official vector sets that are not vendored must use `status: "pending"` and must include a
`pending_reason`. This prevents missing vectors from being mistaken for a successful conformance pass.

## Official Versus Repository Fixtures

Repository fixtures are not official vectors. They are allowed only for harness and parser tests.

The current placeholder fixture is:

```text
test-vectors/placeholders/public-api-failclosed.json
```

It verifies that the vector harness can execute placeholder cases while the public ML-KEM and ML-DSA
APIs remain fail-closed. It does not prove FIPS conformance.

## Validation Script

The manifest validator is:

```bash
python3 scripts/check-vector-manifest.py --self-test
```

It checks:

- malformed JSON is rejected,
- source dates and hashes are present,
- vector-set source links are valid,
- parameter-set names map to known ML-KEM or ML-DSA parameter sets,
- pending official vectors include a clear reason,
- placeholder fixture hashes match the files on disk.

The optional strict gate is:

```bash
python3 scripts/check-vector-manifest.py --require-official
```

That command fails while official vector sets are absent. The normal CI gate does not require official
vectors yet because issue #13 defines the ingestion path, not a completed ACVP validation session.

## C++ Harness

The C++ vector harness lives at:

```text
cpp/include/pqcore/vector_harness.hpp
```

It maps official parameter-set names to internal C++ parameter enums:

- `ML-KEM-512`,
- `ML-KEM-768`,
- `ML-KEM-1024`,
- `ML-DSA-44`,
- `ML-DSA-65`,
- `ML-DSA-87`.

The placeholder runner returns an explicit pending result when a placeholder case reaches a public API
that is still fail-closed. This is intentional: until official vectors and production primitives exist, a
pending official vector is better evidence than a false pass.

## Future Rust Differential Path

Rust remains derivation-only until the Rust crate gate closes. The future differential path should:

1. load the same official vector manifest,
2. run the C++ implementation against each vector,
3. run the Rust implementation against each vector,
4. compare both outputs to the official expected output,
5. compare C++ and Rust outputs to each other,
6. report parameter set, mode, vector-set id, group id, and test-case id for failures.

No Rust crate or Rust vector runner is added in this ticket.

## Readiness Caveat

Official vectors prove conformance for listed cases. They do not replace fuzzing, parser hardening,
constant-time review, entropy review, or external cryptographic review.
