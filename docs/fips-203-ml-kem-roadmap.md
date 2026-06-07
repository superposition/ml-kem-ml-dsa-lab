# FIPS 203 ML-KEM Roadmap

ML-KEM exposes three parameter sets: ML-KEM-512, ML-KEM-768, and ML-KEM-1024.

Source of truth: <https://csrc.nist.gov/pubs/fips/203/final>

Teaching maps:

- Shared glossary: `docs/glossary.md`
- ML-KEM symbols: `docs/fips-203-symbols.md`

## Library Modules

- `params`: names, dimensions, compression widths, and byte lengths. Follow-up: #3.
- `field`: arithmetic modulo `q = 3329`. Follow-up: #4.
- `polynomial`: fixed-size polynomial arithmetic over `Z_q[x] / (x^256 + 1)`. Follow-up: #5.
- `encoding`: byte packing and unpacking. Follow-up: #6.
- `sampling`: centered binomial and rejection sampling. Follow-up: #8.
- `ntt`: forward and inverse transforms. Follow-up: #7.
- `pke`: internal encryption routines. Follow-up: #9.
- `kem`: key generation, encapsulation, and decapsulation. Follow-up: #10.

## TDD Order

1. Parameter metadata tests.
2. Field normalization and arithmetic tests.
3. Polynomial ring arithmetic tests.
4. Encoding round trips.
5. Compression round trips and edge cases.
6. Sampling distribution invariants.
7. NTT round trips.
8. Inner PKE vectors.
9. KEM ACVP vectors.
