# ML-KEM / ML-DSA Lab

From-first-principles C++20 implementation work and Rust derivation track for the NIST module-lattice standards:

- **ML-KEM**, formerly CRYSTALS-Kyber, standardized as FIPS 203.
- **ML-DSA**, formerly CRYSTALS-Dilithium, standardized as FIPS 204.

This repository is designed as a derivation-first implementation lab. Production cryptography status is not claimed yet; it is tracked as an explicit release gate.

## Current Status

| Area | Derived | Tested | Implemented | Reviewed | Notes |
| --- | --- | --- | --- | --- | --- |
| Derivation-first process | Yes | Yes | Yes | Internal | CI blocks premature Rust crates |
| Rust crate | No | No | No | No | Blocked by the Rust crate gate |
| Parameter metadata | Partial | C++ | C++ | No | Needs source-linked derivation notes |
| Finite-field arithmetic | Partial | C++ | C++ | No | Needs expanded derivation notes |
| Polynomial arithmetic | Partial | C++ | C++ | No | Baseline negacyclic multiplication exists |
| ML-KEM keygen/encap/decap | No | No | API only | No | C++ fails closed with `NotImplemented` |
| ML-DSA keygen/sign/verify | No | No | API only | No | C++ fails closed with `NotImplemented` |
| NIST ACVP vectors | No | No | No | No | Required before production claims |

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
bash scripts/check-rust-crate-gate.sh
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

The crate-introduction checklist is tracked in `docs/rust-crate-gate.md`, and CI rejects accidental `rust/**/Cargo.toml` files until that gate is intentionally opened.
