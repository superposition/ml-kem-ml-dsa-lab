# Official Vector Ingestion and Differential Testing

This document defines the vector-ingestion gate for ML-KEM and ML-DSA.

## Source Pointers

| Source | Role | Current repository status |
| --- | --- | --- |
| FIPS 203 final | ML-KEM algorithm source | Referenced for algorithm mapping |
| FIPS 204 final | ML-DSA algorithm source | Referenced for algorithm mapping |
| ACVP ML-KEM JSON Specification | ML-KEM vector-set schema planning | Source manifest recorded |
| ACVP ML-DSA JSON Specification | ML-DSA vector-set schema planning | Source manifest recorded |
| NIST PQC archive | Pointer to CAVP-published ML-KEM and ML-DSA test vectors | Source manifest recorded |
| NIST ACVP-Server `gen-val/json-files` | Generated prompt and expected-results files | Vendored at commit `15c0f3deeefbfa8cb6cd32a99e1ca3b738c66bf0` |

The ACVP pages define JSON schemas and validation-session behavior. The vendored files come from the
NIST ACVP-Server generated validation data published under `gen-val/json-files`. They are pinned by
commit, path, and SHA-256 so the repository can validate provenance without reaching the network.

These files are conformance inputs and expected outputs. They are not evidence that this repository's
public algorithms pass yet; the public APIs still fail closed until the production implementation gates
land.

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
- local prompt and expected-results files,
- per-file SHA-256 hashes,
- group count,
- case count.

Official vector sets that are not vendored must use `status: "pending"` and must include a
`pending_reason`. This prevents missing vectors from being mistaken for a successful conformance pass.
Vendored vector sets must use `status: "vendored"` and must include local files that match the
manifest hashes.

## Vendored ACVP Coverage

| Vector set | Mode | Parameter sets | Groups | Cases |
| --- | --- | --- | --- | --- |
| `ML-KEM-keyGen-FIPS203` | `keyGen` | ML-KEM-512, ML-KEM-768, ML-KEM-1024 | 3 | 75 |
| `ML-KEM-encapDecap-FIPS203` | `encapDecap` | ML-KEM-512, ML-KEM-768, ML-KEM-1024 | 12 | 165 |
| `ML-DSA-keyGen-FIPS204` | `keyGen` | ML-DSA-44, ML-DSA-65, ML-DSA-87 | 3 | 75 |
| `ML-DSA-sigGen-FIPS204` | `sigGen` | ML-DSA-44, ML-DSA-65, ML-DSA-87 | 24 | 360 |
| `ML-DSA-sigVer-FIPS204` | `sigVer` | ML-DSA-44, ML-DSA-65, ML-DSA-87 | 12 | 180 |

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
python3 scripts/check-vector-manifest.py --self-test --require-official
```

It checks:

- malformed JSON is rejected,
- source dates and hashes are present,
- vector-set source links are valid,
- parameter-set names map to known ML-KEM or ML-DSA parameter sets,
- pending official vectors include a clear reason,
- vendored official vectors have prompt and expected-results files,
- vendored official vector file hashes match the manifest,
- vendored ACVP metadata matches algorithm, mode, revision, and sample status,
- vendored prompt files cover every listed parameter set,
- vendored prompt and expected-results files have matching group and case counts,
- placeholder fixture hashes match the files on disk.

CI runs this strict gate. It fails if official vector files are absent, malformed, hash-mismatched, or
silently returned to pending status.

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
that is still fail-closed. This is intentional: until production primitives exist, a pending algorithm
execution is better evidence than a false pass.

The harness also accepts official vector case metadata for the vendored modes:

- ML-KEM `keyGen`,
- ML-KEM `encapDecap`,
- ML-DSA `keyGen`,
- ML-DSA `sigGen`,
- ML-DSA `sigVer`.

For now, official vector case metadata returns a pending result because public algorithm execution and
output comparison are still blocked by the production implementation gates.

## Future Rust Differential Path

The initial Rust crate exists, but it does not yet include public ML-KEM or ML-DSA algorithm
implementations or a vector runner. The future differential path should:

1. load the same official vector manifest,
2. run the C++ implementation against each vector,
3. run the Rust implementation against each vector,
4. compare both outputs to the official expected output,
5. compare C++ and Rust outputs to each other,
6. report parameter set, mode, vector-set id, group id, and test-case id for failures.

No Rust vector runner is added in this ticket. The Rust differential gate remains future work.

## Readiness Caveat

Vendored official vectors make conformance testing reproducible. They do not prove this repository's
algorithms pass until the public implementations execute them successfully, and they do not replace
fuzzing, parser hardening, constant-time review, entropy review, or external cryptographic review.
