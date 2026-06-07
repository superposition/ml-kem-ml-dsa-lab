# FIPS 204 ML-DSA Roadmap

ML-DSA exposes three parameter sets: ML-DSA-44, ML-DSA-65, and ML-DSA-87.

## Library Modules

- `params`: dimensions, bounds, challenge strength, and byte lengths.
- `field`: arithmetic modulo `q = 8380417`.
- `polynomial`: fixed-size polynomial arithmetic over `Z_q[x] / (x^256 + 1)`.
- `encoding`: key and signature packing.
- `sampling`: secret, mask, and challenge sampling.
- `ntt`: forward and inverse transforms.
- `rounding`: Power2Round, Decompose, HighBits, LowBits, MakeHint, UseHint.
- `dsa`: key generation, signing, and verification.

## TDD Order

1. Parameter metadata tests.
2. Field normalization and arithmetic tests.
3. Polynomial ring arithmetic tests.
4. Encoding round trips.
5. Rounding and hint invariants.
6. Sampling invariants.
7. NTT round trips.
8. Signature ACVP vectors.

