# Standards Map

## Source Of Truth

- FIPS 203: <https://csrc.nist.gov/pubs/fips/203/final>
- FIPS 204: <https://csrc.nist.gov/pubs/fips/204/final>
- ACVP ML-KEM JSON draft: <https://pages.nist.gov/ACVP/draft-celi-acvp-ml-kem.html>
- ACVP ML-DSA JSON draft: <https://pages.nist.gov/ACVP/draft-celi-acvp-ml-dsa.html>

FIPS 203 and FIPS 204 are the algorithm sources. ACVP material is for vector and validation workflow planning.

Both FIPS pages include planning-note or errata references as of June 7, 2026. Re-check the NIST pages before implementing or reviewing algorithm details.

## Naming

- Kyber is standardized as **ML-KEM** in FIPS 203.
- Dilithium is standardized as **ML-DSA** in FIPS 204.

The public API uses the standardized names. Legacy names can appear in documentation and compatibility notes.

Do not assume final FIPS ML-KEM is byte-compatible with older Kyber drafts, and do not assume final FIPS ML-DSA is byte-compatible with older Dilithium drafts. Any compatibility claim needs an exact revision and tests.

## Glossary And Symbols

- Shared glossary: `docs/glossary.md`
- ML-KEM symbol map: `docs/fips-203-symbols.md`
- ML-DSA symbol map: `docs/fips-204-symbols.md`

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

See `docs/fips-203-symbols.md` for the module-to-symbol map and issue links.

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

See `docs/fips-204-symbols.md` for the module-to-symbol map and issue links.
