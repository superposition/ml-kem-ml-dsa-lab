# ML-KEM / ML-DSA Lab

From-first-principles C++20 implementation work and Rust derivation track for the NIST module-lattice standards:

- **ML-KEM**, formerly CRYSTALS-Kyber, standardized as FIPS 203.
- **ML-DSA**, formerly CRYSTALS-Dilithium, standardized as FIPS 204.

This repository is designed as a derivation-first implementation lab. Production cryptography status is not claimed yet; it is tracked as an explicit release gate.

## Current Status

| Area | Derived | Tested | Implemented | Reviewed | Notes |
| --- | --- | --- | --- | --- | --- |
| Derivation-first process | Yes | Yes | Yes | Internal | CI validates the intentional internal Rust crate shape |
| Rust crate | Partial | Cargo test | Internal derivation crate | Internal | No public ML-KEM/ML-DSA API; not production cryptography |
| Parameter metadata | Yes | C++ + fixture | C++ | Internal | Source-linked derivation and shared fixture exist |
| Finite-field arithmetic | Yes | C++ | C++ | Internal | Source-linked derivation and arithmetic tests exist |
| Polynomial arithmetic | Yes | C++ | C++ | Internal | Source-linked derivation and schoolbook oracle tests exist |
| Encoding and compression | Yes | C++ + fixture | C++ | Internal | ML-KEM byte packing and compression derivation exist |
| NTT representation | Yes | C++ + fixture | C++ | Internal | ML-KEM NTT round trips and schoolbook oracle tests exist |
| Sampling and deterministic hooks | Yes | C++ + fixture | C++ | Internal | Test-only expansion, rejection, and bounded samplers exist |
| Internal ML-KEM PKE | Partial | C++ + fixture | C++ | Internal | ML-KEM-512 test-hook PKE layer exists |
| ML-KEM keygen/encap/decap | Partial | C++ + fixture | Internal test hook | Internal | ML-KEM-512 internal KEM path exists; public API fails closed until vectors land |
| ML-DSA rounding/hints/challenge | Yes | C++ + fixture | C++ helpers | Internal | Power2Round, Decompose, hints, and challenge construction exist |
| ML-DSA keygen/sign/verify | Partial | C++ + fixture | Internal test hook | Internal | Signing-flow harness exists; public API fails closed until vectors land |
| NIST ACVP vectors | Partial | Manifest + harness | Parser + pending gate | Internal | Official sources recorded; vector sets pending |
| Side-channel review gate | Partial | Manifest + C++ redaction test | Review gate | Internal | Production status remains false; blockers are tracked |

## Repository Layout

```text
.
├── cpp/                         # C++20 header-first library and CTest suite
├── rust/                        # Rust derivation track plus internal pqcore crate
├── docs/                        # Standards map, implementation roadmap, readiness gates
├── fixtures/                    # Repository-owned shared test fixtures
├── schemas/                     # Agentic learning trace schema
├── learning/                    # Initial learning tracks for ML-KEM and ML-DSA
└── test-vectors/                # Placeholder for ACVP/KAT vector ingestion
```

## Test Commands

```bash
cmake -S cpp -B build/cpp -DPQCORE_BUILD_TESTS=ON
cmake --build build/cpp
ctest --test-dir build/cpp --output-on-failure
cargo test --manifest-path rust/pqcore/Cargo.toml
python3 -m json.tool schemas/agentic-learning.schema.json >/dev/null
python3 -m json.tool learning/ml-kem-fips203.track.json >/dev/null
python3 -m json.tool learning/ml-dsa-fips204.track.json >/dev/null
python3 -m json.tool fixtures/parameter-sets.json >/dev/null
bash scripts/check-docs-glossary.sh
python3 scripts/check-vector-manifest.py --self-test
python3 scripts/check-side-channel-manifest.py --self-test
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
- `docs/glossary.md` explains shared notation before implementation work.
- `docs/parameter-tables.md` derives the shared parameter metadata.
- `docs/field-elements.md` derives canonical and centered field representation.
- `docs/polynomial-ring.md` derives polynomial arithmetic over `R_q`.
- `docs/encoding-and-compression.md` derives ML-KEM byte encoding and compression.
- `docs/ntt.md` derives the ML-KEM NTT representation and oracle tests.
- `docs/sampling.md` derives sampling boundaries and deterministic test hooks.
- `docs/ml-kem-pke.md` derives the internal ML-KEM-512 PKE layer.
- `docs/ml-kem-kem.md` derives the internal ML-KEM-512 KEM path and public fail-closed contract.
- `docs/ml-dsa-rounding-hints.md` derives ML-DSA rounding, hints, and challenge construction.
- `docs/ml-dsa-signature.md` derives the ML-DSA signature API contract and internal signing-flow boundary.
- `docs/vector-ingestion.md` defines official vector provenance, pending behavior, and future differential tests.
- `docs/side-channel-review.md` defines secret-bearing surfaces and the constant-time review gate.
- `docs/fips-203-symbols.md` maps ML-KEM symbols and modules.
- `docs/fips-204-symbols.md` maps ML-DSA symbols and modules.

## Rust Policy

The initial Rust crate lives at `rust/pqcore`. It is an internal derivation crate with private modules
for parameter tables, field elements, and polynomial arithmetic. It intentionally exposes no public
ML-KEM or ML-DSA algorithm API.

The crate-introduction checklist is tracked in `docs/rust-crate-gate.md`, and CI rejects extra or
public-facing Rust crate surfaces that bypass the issue #15 boundary.
