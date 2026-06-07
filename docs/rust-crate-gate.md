# Rust Crate Gate

The initial Rust crate now lives at:

```text
rust/pqcore
```

It is an internal derivation crate, not a production cryptography crate. It exposes no public ML-KEM or
ML-DSA algorithm API.

The Rust implementation should be derived before it is packaged. The crate is allowed only because the
foundational module boundaries, invariants, and tests are clear enough that the package shape follows
the derivations rather than replacing them.

## Current Rule

After issue #15, CI must fail unless exactly one crate manifest exists:

```text
rust/pqcore/Cargo.toml
```

The crate must remain `publish = false`, its README must avoid production claims, and it must not expose
public ML-KEM or ML-DSA API stubs.

The guard lives in:

```text
scripts/check-rust-crate-gate.sh
```

## Crate Introduction Checklist

Evidence for the issue #15 crate opening:

- parameter table derivation and tests: `docs/parameter-tables.md`, `rust/pqcore/src/params.rs`,
- field element derivation and tests: `docs/field-elements.md`, `rust/pqcore/src/field.rs`,
- polynomial ring derivation and tests: `docs/polynomial-ring.md`, `rust/pqcore/src/polynomial.rs`,
- encoding and decoding rules: `docs/encoding-and-compression.md`,
- vector ingestion strategy: `docs/vector-ingestion.md`, `test-vectors/manifest.json`,
- secret-bearing type policy: `docs/side-channel-review.md`,
- constant-time coding rules: `audits/side-channel/manifest.json`,
- cross-language fixture format: `fixtures/parameter-sets.json`, `fixtures/polynomial-examples.json`,
- public API naming policy that uses ML-KEM and ML-DSA: `docs/fips-203-symbols.md`, `docs/fips-204-symbols.md`.

## Future Public API Protocol

1. Keep new modules private until their derivation tests exist.
2. Add shared fixture or official-vector evidence before exporting behavior.
3. Preserve ML-KEM and ML-DSA names in public APIs; do not expose Kyber/Dilithium-only names.
4. Keep production status false until official vectors, side-channel review, entropy review, fuzzing,
   and external review are complete.
5. Update the crate-shape guard in the same change that intentionally broadens the public surface.

## Teaching Note

Rust improves memory safety, but it does not prove cryptographic correctness. The crate gate keeps the project focused on algebra, standard conformance, test vectors, and side-channel discipline before packaging.
