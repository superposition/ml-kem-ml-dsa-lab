# FIPS 204 ML-DSA Roadmap

ML-DSA exposes three parameter sets: ML-DSA-44, ML-DSA-65, and ML-DSA-87.

Source of truth: <https://csrc.nist.gov/pubs/fips/204/final>

Teaching maps:

- Shared glossary: `docs/glossary.md`
- ML-DSA symbols: `docs/fips-204-symbols.md`

## Library Modules

- `params`: dimensions, bounds, challenge strength, and byte lengths. Follow-up: #3.
- `field`: arithmetic modulo `q = 8380417`. Follow-up: #4.
- `polynomial`: fixed-size polynomial arithmetic over `Z_q[x] / (x^256 + 1)`. Follow-up: #5.
- `encoding`: key and signature packing. Follow-up: #6.
- `sampling`: secret, mask, and challenge sampling. Follow-up: #8.
- `ntt`: forward and inverse transforms. Follow-up: #7.
- `rounding`: Power2Round, Decompose, HighBits, LowBits, MakeHint, UseHint. Follow-up: #11.
- `dsa`: key generation, signing, and verification. Follow-up: #12.

## TDD Order

1. Parameter metadata tests.
2. Field normalization and arithmetic tests.
3. Polynomial ring arithmetic tests.
4. Encoding round trips.
5. Rounding and hint invariants.
6. Sampling invariants.
7. NTT round trips.
8. Signature ACVP vectors.
