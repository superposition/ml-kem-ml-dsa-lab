# Rust Derivation: ML-DSA Key Generation, Signing, and Verification

There is intentionally no Rust crate for this work yet.

The future Rust implementation should derive the signature API from `docs/ml-dsa-signature.md` and
`fixtures/ml-dsa-signature-examples.json`.

## Future Type Shape

Public API:

```text
MlDsaKeyPair
Signature
ml_dsa_keygen(parameter_set)
ml_dsa_sign(parameter_set, secret_key, message, context)
ml_dsa_verify(parameter_set, public_key, message, signature, context)
```

Internal test/validation API:

```text
ml_dsa_keygen_internal_test(parameter_set, seed)
ml_dsa_sign_internal_test(parameter_set, secret_key, formatted_message, signing_random)
ml_dsa_verify_internal_test(parameter_set, public_key, formatted_message, signature)
ml_dsa_format_message(message, context)
```

The internal test API mirrors the signing-flow boundaries while the production public API remains
fail-closed.

## Future Red Tests

1. Public key generation remains fail-closed until official vectors exist.
2. Public signing rejects malformed secret-key lengths.
3. Public signing rejects contexts longer than 255 bytes.
4. Public verification rejects malformed public-key lengths.
5. Public verification rejects malformed signature lengths.
6. Public verification rejects contexts longer than 255 bytes.
7. Internal key generation returns exact key lengths for ML-DSA-44, ML-DSA-65, and ML-DSA-87.
8. Internal key generation stores matching `rho` in the public key and secret key header.
9. Internal key generation stores `tr = H(pk, 64)` in the secret key header.
10. Message formatting produces `00 || context_length || context || message`.
11. Internal signing returns exact signature lengths for every parameter set.
12. Internal signing exercises a deterministic rejection-loop fixture.
13. Internal verification accepts the matching deterministic signature.
14. Internal verification rejects modified messages.
15. Internal verification rejects modified public keys.
16. Internal verification rejects modified signatures.
17. Internal verification rejects response-bound failures.
18. Internal verification rejects too many hints.
19. Normal builds cannot call deterministic internal signature hooks accidentally.
20. No Rust crate is introduced until the crate gate is intentionally opened.

## Readiness Caveat

The current internal C++ path is a signing-flow harness, not an ML-DSA implementation. Production
readiness still requires ML-DSA encoding, ML-DSA NTT, SHAKE/XOF/PRF plumbing, official vectors,
constant-time review, fuzzing, and external cryptographic review.
