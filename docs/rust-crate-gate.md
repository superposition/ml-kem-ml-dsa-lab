# Rust Crate Gate

There is intentionally no Rust crate in this repository yet.

The Rust implementation should be derived before it is packaged. A crate can be introduced only after the foundational module boundaries, invariants, and tests are clear enough that the public API is a result of understanding rather than a substitute for it.

## Current Rule

Until issue #15 is intentionally completed, CI must fail if `rust/**/Cargo.toml` appears.

The guard lives in:

```text
scripts/check-rust-crate-gate.sh
```

## Crate Introduction Checklist

Before adding a Rust `Cargo.toml`, complete and link evidence for:

- parameter table derivation and tests,
- field element derivation and tests,
- polynomial ring derivation and tests,
- encoding and decoding rules,
- vector ingestion strategy,
- secret-bearing type policy,
- constant-time coding rules,
- cross-language fixture format,
- public API naming policy that uses ML-KEM and ML-DSA.

## Unblocking Protocol

1. Complete the derivation tickets that define the Rust module boundaries.
2. Update this document with links to the evidence.
3. Open a PR or commit that explicitly references issue #15.
4. Remove or revise the crate-gate check in the same change that introduces the first Rust crate.
5. Add Rust CI only after the first crate has real derivation tests.

## Teaching Note

Rust improves memory safety, but it does not prove cryptographic correctness. The crate gate keeps the project focused on algebra, standard conformance, test vectors, and side-channel discipline before packaging.

