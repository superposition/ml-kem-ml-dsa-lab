# Production Readiness Gates

Production cryptography has not yet been determined for this repository. A production-ready release requires all gates below to be complete and documented.

## Gate 1: Spec Coverage

- ML-KEM key generation, encapsulation, and decapsulation are implemented from FIPS 203.
- ML-DSA key generation, signing, and verification are implemented from FIPS 204.
- Every public algorithm has tests that point to a standard section, vector source, or derived invariant.

## Gate 2: Official Vectors

- ACVP/KAT vectors are ingested reproducibly.
- Vector provenance is documented.
- Rust and C++ implementations pass the same vectors.

## Gate 3: Constant-Time Discipline

- Secret-dependent branches are removed or justified.
- Secret-dependent memory access is removed or justified.
- Compiler behavior is reviewed for optimized builds.
- Timing tests are advisory only; code review is required.
- Current gate state is tracked in `docs/side-channel-review.md` and `audits/side-channel/manifest.json`.
- Production status remains false while the manifest status is `incomplete-production-blocker`.

## Gate 4: Entropy and RNG

- Key generation and randomized signing use reviewed entropy sources.
- Deterministic test hooks cannot be enabled accidentally in release builds.
- Failure modes are explicit and fail closed.

## Gate 5: Memory Safety

- Rust code avoids `unsafe` unless reviewed.
- C++ code runs under sanitizers in CI.
- Fuzz targets cover decoders, decompression, and public parsers.

## Gate 6: External Review

- At least one external cryptographic review is completed.
- Findings are tracked publicly.
- Production-readiness claims are tied to a signed release tag.
