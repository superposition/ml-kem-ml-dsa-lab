# ML-KEM / ML-DSA Lab

From-first-principles C++20 implementation work and Rust derivation track for the NIST module-lattice standards:

- **ML-KEM**, formerly CRYSTALS-Kyber, standardized as FIPS 203.
- **ML-DSA**, formerly CRYSTALS-Dilithium, standardized as FIPS 204.

This repository is designed as a derivation-first implementation lab. Production cryptography status is not claimed yet; it is tracked as an explicit release gate.

## Current Status

| Area | Rust | C++ | Notes |
| --- | --- | --- | --- |
| Parameter metadata | Derivation planned | Implemented | ML-KEM-512/768/1024 and ML-DSA-44/65/87 |
| Finite-field arithmetic | Derivation planned | Implemented | Modular arithmetic primitives with unit tests |
| Polynomial arithmetic | Derivation planned | Implemented | Negacyclic schoolbook multiplication for baseline testing |
| ML-KEM keygen/encap/decap | Not started | API only | C++ fails closed with `NotImplemented` |
| ML-DSA keygen/sign/verify | Not started | API only | C++ fails closed with `NotImplemented` |
| NIST ACVP vectors | Planned | Planned | Required before production claims |

## Repository Layout

```text
.
├── cpp/                         # C++20 header-first library and CTest suite
├── rust/                        # Rust derivation track; no crate yet
├── docs/                        # Standards map, implementation roadmap, readiness gates
├── schemas/                     # Agentic learning trace schema
├── learning/                    # Initial learning tracks for ML-KEM and ML-DSA
└── test-vectors/                # Placeholder for ACVP/KAT vector ingestion
```

## Test Commands

```bash
cmake -S cpp -B build/cpp -DPQCORE_BUILD_TESTS=ON
cmake --build build/cpp
ctest --test-dir build/cpp --output-on-failure
python3 -m json.tool schemas/agentic-learning.schema.json >/dev/null
python3 -m json.tool learning/ml-kem-fips203.track.json >/dev/null
python3 -m json.tool learning/ml-dsa-fips204.track.json >/dev/null
```

Or run both:

```bash
make test
```

## Production Readiness

This library must not be treated as production cryptography until the production readiness gates in `docs/production-readiness.md` are complete. At minimum, that means:

- Passing official FIPS 203 and FIPS 204 vector suites.
- Constant-time and side-channel review for secret-dependent paths.
- RNG/entropy policy review.
- Fuzzing, sanitizers, and cross-language differential testing.
- External cryptographic review.
- Versioned API and release policy.

## Standards

- FIPS 203: Module-Lattice-Based Key-Encapsulation Mechanism Standard.
- FIPS 204: Module-Lattice-Based Digital Signature Standard.
- NIST ACVP vector generation/validation data is the planned test-vector source.

## Rust Policy

There is intentionally no Rust crate yet. Rust should be derived through notes, red tests, and module-level invariants before a library surface is introduced.
