# pqcore Rust Crate

This is the initial internal Rust crate for the ML-KEM / ML-DSA derivation lab.

It is not production cryptography. It has no public ML-KEM or ML-DSA API yet, and it must not be
used for key generation, encapsulation, decapsulation, signing, or verification.

The first crate tests are derivation tests for:

- parameter tables,
- field-element arithmetic,
- polynomial ring arithmetic,
- shared repository fixture links,
- vector manifest parsing and pending-status reporting.

The `rust-vector-runner` binary loads `test-vectors/manifest.json`, reads vendored ACVP prompt and
expected-results case ids, and reports unavailable Rust algorithm execution as pending. It does not
print raw vector seed, key, ciphertext, signature, or shared-secret fields.

Production readiness remains blocked on official vectors, constant-time review, entropy review,
fuzzing, sanitizer coverage, and external cryptographic review.
