# FIPS 203 ML-KEM Roadmap

ML-KEM exposes three parameter sets: ML-KEM-512, ML-KEM-768, and ML-KEM-1024.

## Library Modules

- `params`: names, dimensions, compression widths, and byte lengths.
- `field`: arithmetic modulo `q = 3329`.
- `polynomial`: fixed-size polynomial arithmetic over `Z_q[x] / (x^256 + 1)`.
- `encoding`: byte packing and unpacking.
- `sampling`: centered binomial and rejection sampling.
- `ntt`: forward and inverse transforms.
- `pke`: internal encryption routines.
- `kem`: key generation, encapsulation, and decapsulation.

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

