# Rust Derivation: ML-DSA Rounding, Hints, and Challenge

There is intentionally no Rust crate for this work yet.

The future Rust implementation should derive these helpers from
`docs/ml-dsa-rounding-hints.md` and `fixtures/ml-dsa-helper-examples.json`.

## Future Type Shape

The helper surface should stay below the public signature API:

```text
MlDsaDecomposition { high, low }
ml_dsa_positive_mod(value, modulus)
ml_dsa_centered_mod(value, modulus)
ml_dsa_power2round(parameter_set, r)
ml_dsa_decompose(parameter_set, r)
ml_dsa_high_bits(parameter_set, r)
ml_dsa_low_bits(parameter_set, r)
ml_dsa_make_hint(parameter_set, z, r)
ml_dsa_use_hint(parameter_set, hint, r)
ml_dsa_challenge_polynomial_from_bytes(parameter_set, bytes)
```

The challenge helper models the post-XOF byte stream for `SampleInBall`. A production SHAKE
path and official vectors remain later gates.

## Future Red Tests

1. `Power2Round` recomposes to the original value modulo `q`.
2. `Power2Round(4097)` returns high `1` and low `-4095` for every ML-DSA set.
3. `Decompose` recomposes to the original value modulo `q`.
4. `Decompose(q - 1)` returns `(0, -1)` for every ML-DSA set.
5. `HighBits` returns the high part from `Decompose`.
6. `LowBits` returns the low part from `Decompose`.
7. `MakeHint(1, 0)` returns false for every ML-DSA set.
8. `MakeHint(1, gamma2)` returns true for every ML-DSA set.
9. `UseHint(false, r)` returns `HighBits(r)`.
10. `UseHint(true, gamma2)` increments the high bits.
11. `UseHint(true, 0)` wraps to the last high-bit bucket.
12. Challenge polynomial construction returns exactly `tau` nonzero coefficients.
13. Challenge coefficients are only `-1`, `0`, or `1`.
14. Challenge coverage is parameterized for ML-DSA-44, ML-DSA-65, and ML-DSA-87.
15. Public signing and verification APIs remain fail-closed until their ticket.

## Readiness Caveat

These helpers are correctness-critical but do not prove signature security. Production readiness
still requires full ML-DSA key generation, signing, verification, official vectors, constant-time
review, and external cryptographic review.
