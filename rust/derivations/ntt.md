# Rust Derivation: NTT

There is intentionally no Rust crate for this work yet.

The future Rust implementation should derive NTT behavior from `docs/ntt.md` and the shared fixture at `fixtures/ntt-examples.json`.

## Future Type Shape

Coefficient-domain and NTT-domain polynomials should be distinct types:

```text
MlKemPolynomial
MlKemNttPolynomial
```

The compiler should prevent NTT-domain multiplication from accepting coefficient-domain values.

## Future Red Tests

1. `BitRev7(0) = 0`.
2. `BitRev7(1) = 64`.
3. `zeta^BitRev7(1) mod 3329 = 1729` for ML-KEM.
4. `zeta^(2*BitRev7(0)+1) mod 3329 = 17` for ML-KEM base multiplication.
5. The zero polynomial stays zero under forward NTT.
6. The zero NTT polynomial inverse-transforms to zero.
7. A constant polynomial forward-transforms into the documented pair ordering.
8. `InverseNTT(NTT(f)) = f` for representative ML-KEM polynomials.
9. `InverseNTT(NTT(f) * NTT(g)) = schoolbook_negacyclic(f, g)`.
10. Coefficient-domain and NTT-domain types cannot be mixed silently.
11. The shared NTT fixture parses without introducing a production vector claim.

## ML-DSA Follow-Up

Future ML-DSA NTT work should use FIPS 204 Sections 7.5 and 7.6, the ML-DSA modulus `8380417`, and root `1753`. It should not reuse ML-KEM constants.

## Readiness Caveat

NTT tests prove functional representation behavior only. Secret-bearing NTT paths still need constant-time and side-channel review.

