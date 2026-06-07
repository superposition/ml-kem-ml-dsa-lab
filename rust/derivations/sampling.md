# Rust Derivation: Sampling

The initial Rust crate exists, but this module remains future derivation work.

The future Rust implementation should derive sampling from `docs/sampling.md` and the shared fixture at `fixtures/sampling-examples.json`.

## Future Type Shape

Deterministic test expansion must be visibly separate from production entropy:

```text
test_expand(seed, domain, nonce, output_len)
production_random_bytes(output_len)
```

The test function should be compiled only for tests. The production entropy path should fail closed until a reviewed source is wired.

## Future Red Tests

1. A fixed deterministic test seed produces the fixture byte prefix.
2. Changing the nonce changes deterministic output.
3. Changing the domain separator changes deterministic output.
4. ML-KEM PRF test expansion produces `64 * eta` bytes for `eta = 2` and `eta = 3`.
5. ML-KEM `SampleNTT` rejects candidates greater than or equal to `3329`.
6. ML-KEM `SampleNTT` emits only coefficients in `[0, 3329)`.
7. ML-KEM `SamplePolyCBD_eta` accepts exactly `64 * eta` bytes.
8. ML-KEM CBD centered coefficients stay in `[-eta, eta]`.
9. ML-DSA `CoeffFromThreeBytes` rejects values greater than or equal to `8380417`.
10. ML-DSA `CoeffFromHalfByte` rejects unmapped nibbles.
11. ML-DSA bounded sampling emits centered coefficients in `[-eta, eta]`.
12. ML-DSA `SampleInBall` emits exactly `tau` nonzero coefficients.
13. The default production build cannot call test expansion accidentally.

## Production Boundary

Rust memory safety does not make sampling safe. The final implementation still needs FIPS-compatible SHAKE/XOF/PRF code, official vectors, entropy review, and side-channel review.

## Readiness Caveat

Distribution sanity checks are not proof of correct sampling. The implementation must follow the standard algorithms and pass official vector coverage before production claims.
