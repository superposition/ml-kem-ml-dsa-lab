# ML-KEM / ML-DSA Lab

From-first-principles C++20 and Rust implementations of the NIST module-lattice standards:

- **ML-KEM**, formerly CRYSTALS-Kyber, standardized as FIPS 203.
- **ML-DSA**, formerly CRYSTALS-Dilithium, standardized as FIPS 204.

This repository is designed as a publishable library and a test-driven implementation lab. Production cryptography status is not claimed yet; it is tracked as an explicit release gate.

## Current Status

| Area | Rust | C++ | Notes |
| --- | --- | --- | --- |
| Parameter metadata | Implemented | Implemented | ML-KEM-512/768/1024 and ML-DSA-44/65/87 |
| Finite-field arithmetic | Implemented | Implemented | Modular arithmetic primitives with unit tests |
| Polynomial arithmetic | Implemented | Implemented | Negacyclic schoolbook multiplication for baseline testing |
| ML-KEM keygen/encap/decap | API only | API only | Fails closed with `NotImplemented` |
| ML-DSA keygen/sign/verify | API only | API only | Fails closed with `NotImplemented` |
| NIST ACVP vectors | Planned | Planned | Required before production claims |

## Repository Layout

```text
.
├── cpp/                         # C++20 header-first library and CTest suite
├── rust/pqcore/                 # Rust library crate
├── docs/                        # Standards map, implementation roadmap, readiness gates
├── schemas/                     # Agentic learning trace schema
├── learning/                    # Initial learning tracks for ML-KEM and ML-DSA
└── test-vectors/                # Placeholder for ACVP/KAT vector ingestion
```

## Test Commands

```bash
cargo test --workspace
cmake -S cpp -B build/cpp -DPQCORE_BUILD_TESTS=ON
cmake --build build/cpp
ctest --test-dir build/cpp --output-on-failure
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

