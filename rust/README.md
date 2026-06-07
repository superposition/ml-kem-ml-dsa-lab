# Rust Derivation Track

There is intentionally no Rust library crate here yet.

The Rust implementation should be derived module by module from:

1. the FIPS text,
2. small mathematical invariants,
3. red tests that describe the next behavior,
4. a minimal green implementation,
5. cross-language agreement with the C++ derivation once both sides exist.

The crate should be introduced only after the foundational derivation tickets are complete. That keeps the project from pretending that API shape is the same thing as cryptographic understanding.

## Crate Creation Gate

Do not add `Cargo.toml` or a public Rust API until the checklist in `../docs/rust-crate-gate.md` is satisfied. At minimum, these must be documented:

- parameter tables and byte lengths,
- field element semantics,
- polynomial ring semantics,
- encoding and decoding rules,
- vector ingestion strategy,
- constant-time coding rules for secret-bearing operations.

CI runs `scripts/check-rust-crate-gate.sh` and fails if a Rust `Cargo.toml` appears before the gate is intentionally opened.
