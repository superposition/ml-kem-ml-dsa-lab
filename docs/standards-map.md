# Standards Map

## Naming

- Kyber is standardized as **ML-KEM** in FIPS 203.
- Dilithium is standardized as **ML-DSA** in FIPS 204.

The public API uses the standardized names. Legacy names can appear in documentation and compatibility notes.

## ML-KEM Scope

ML-KEM is a key encapsulation mechanism. The planned implementation order is:

1. Parameter tables.
2. Byte encoding and decoding.
3. Compression and decompression.
4. Sampling.
5. NTT and inverse NTT.
6. Inner public-key encryption routines.
7. Key generation, encapsulation, and decapsulation.
8. ACVP vector ingestion.
9. Constant-time review.

## ML-DSA Scope

ML-DSA is a digital signature algorithm. The planned implementation order is:

1. Parameter tables.
2. Byte encoding and decoding.
3. Sampling.
4. NTT and inverse NTT.
5. Power2Round, Decompose, HighBits, LowBits, and hint routines.
6. Key generation.
7. Signing.
8. Verification.
9. ACVP vector ingestion.
10. Constant-time review.

