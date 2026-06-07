# Rust Derivation Track

The initial internal Rust crate lives at:

```text
rust/pqcore
```

It is not production cryptography and exposes no public ML-KEM or ML-DSA algorithm API yet.

The Rust implementation should be derived module by module from:

1. the FIPS text,
2. small mathematical invariants,
3. red tests that describe the next behavior,
4. a minimal green implementation,
5. cross-language agreement with the C++ derivation once both sides exist.

The crate was introduced only after the foundational derivation tickets established module boundaries.
That keeps the project from pretending that API shape is the same thing as cryptographic
understanding.

## Crate Creation Gate

Do not add another `Cargo.toml` or a public Rust API without updating the checklist in
`../docs/rust-crate-gate.md`. The current crate shape is limited to derivation tests for:

- parameter tables and byte lengths,
- field element semantics,
- polynomial ring semantics,
- shared repository fixture links.

CI runs `scripts/check-rust-crate-gate.sh` and `cargo test --manifest-path rust/pqcore/Cargo.toml`.
