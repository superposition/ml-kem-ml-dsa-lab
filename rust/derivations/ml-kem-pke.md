# Rust Derivation: ML-KEM Internal PKE

The initial Rust crate exists, but this module remains future derivation work.

The future Rust implementation should derive internal K-PKE behavior from `docs/ml-kem-pke.md` and `fixtures/ml-kem-pke-examples.json`.

## Future Type Shape

Internal PKE should remain separate from the public KEM API:

```text
MlKemPkeKeyPair
MlKemPkeCiphertext
ml_kem_pke_keygen_512_test(seed)
ml_kem_pke_encrypt_512_test(ek, message, coins)
ml_kem_pke_decrypt_512(dk, ciphertext)
```

The `_test` path should depend on test-only deterministic expansion until real SHAKE plumbing lands.

## Future Red Tests

1. Matrix generation from `rho` is deterministic.
2. Matrix shape is `2 x 2` for ML-KEM-512.
3. Secret and error vectors have dimension `2`.
4. Secret and error coefficients stay inside the centered CBD bound.
5. Internal key generation outputs an 800-byte encryption key and 768-byte decryption key.
6. Malformed encryption-key lengths are rejected.
7. Encryption with fixed coins produces a stable ciphertext fixture.
8. Decryption recovers a controlled 32-byte message.
9. Malformed ciphertext and decryption-key lengths are rejected.
10. Public ML-KEM keygen/encaps/decaps APIs remain separate from internal PKE tests.
11. Normal builds cannot use the deterministic K-PKE test hook accidentally.

## Readiness Caveat

Internal K-PKE test fixtures are not public ML-KEM vectors. Production readiness still requires FIPS-compatible SHAKE/XOF/PRF, official vectors, the outer KEM transform, entropy review, and constant-time review.
