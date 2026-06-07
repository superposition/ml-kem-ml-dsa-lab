# Rust Derivation: ML-KEM KEM

The initial Rust crate exists, but this module remains future derivation work.

The future Rust implementation should derive KEM behavior from `docs/ml-kem-kem.md` and
`fixtures/ml-kem-kem-examples.json`.

## Future Type Shape

The public KEM API should remain separate from the internal deterministic test hooks:

```text
MlKemKeyPair
MlKemCiphertext
SharedSecret
ml_kem_keygen(parameter_set)
ml_kem_encapsulate(parameter_set, public_key)
ml_kem_decapsulate(parameter_set, secret_key, ciphertext)

ml_kem_keygen_internal_512_test(d, z)
ml_kem_encaps_internal_512_test(ek, m)
ml_kem_decaps_internal_512_test(dk, c)
```

The `_test` functions model FIPS `KeyGen_internal`, `Encaps_internal`, and `Decaps_internal`
until real SHA3/SHAKE and official vectors land.

## Future Red Tests

1. Internal ML-KEM-512 key generation returns an 800-byte encapsulation key and 1632-byte decapsulation key.
2. The decapsulation key layout is `dkPKE || ek || H(ek) || z`.
3. Internal encapsulation rejects malformed encapsulation-key lengths.
4. Internal encapsulation returns a 768-byte ciphertext and 32-byte shared secret.
5. Internal decapsulation rejects malformed decapsulation-key lengths.
6. Internal decapsulation rejects malformed ciphertext lengths.
7. A fixed deterministic internal keygen fixture has stable key prefixes.
8. A fixed deterministic internal encapsulation fixture has stable ciphertext and shared-secret prefixes.
9. Decapsulating the matching ciphertext recovers the encapsulated shared secret.
10. Decapsulating a corrupted but well-sized ciphertext returns a deterministic fallback secret.
11. Secret selection uses mask-based selection rather than a branch on the validation flag.
12. Normal builds cannot use deterministic internal KEM hooks accidentally.
13. Public ML-KEM API stubs remain fail-closed until official vectors and real hashing land.

## Readiness Caveat

Internal KEM test fixtures are not public ML-KEM vectors. Production readiness still requires
FIPS-compatible SHA3/SHAKE/XOF/PRF, official ACVP/KAT vectors, entropy review, constant-time
review, and external cryptographic review.
