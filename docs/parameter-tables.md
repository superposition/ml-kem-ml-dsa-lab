# Parameter Table Derivation

This document derives the repository parameter tables from the FIPS sources. It is a teaching artifact and a checklist for implementation tests.

## Source Pointers

| Scheme | FIPS source | Table source | Size source | Security source |
| --- | --- | --- | --- | --- |
| ML-KEM | FIPS 203 | Table 2 | Table 3 | Text after Table 3 |
| ML-DSA | FIPS 204 | Table 1 | Table 2 | Table 1 and text after Table 1 |

## Why Start Here

Parameter tables are deterministic. They are the safest first implementation surface because they do not require randomness, parsing, NTT, or secret-dependent control flow.

They still matter cryptographically. A wrong byte length or dimension makes official vectors impossible to pass and can cause later code to parse keys, ciphertexts, or signatures incorrectly.

## ML-KEM Parameters

FIPS 203 Table 2 defines `n`, `q`, `k`, `eta1`, `eta2`, `du`, `dv`, and required RBG strength. FIPS 203 Table 3 defines key and ciphertext sizes. The text after Table 3 states the claimed security categories.

| Parameter set | n | q | k | eta1 | eta2 | du | dv | RBG strength | Category | ek bytes | dk bytes | ct bytes | ss bytes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| ML-KEM-512 | 256 | 3329 | 2 | 3 | 2 | 10 | 4 | 128 | 1 | 800 | 1632 | 768 | 32 |
| ML-KEM-768 | 256 | 3329 | 3 | 2 | 2 | 10 | 4 | 192 | 3 | 1184 | 2400 | 1088 | 32 |
| ML-KEM-1024 | 256 | 3329 | 4 | 2 | 2 | 11 | 5 | 256 | 5 | 1568 | 3168 | 1568 | 32 |

Teaching notes:

- `n = 256` means each polynomial has 256 coefficients.
- `q = 3329` is the modulus for every coefficient.
- `k` controls vector and matrix size, which is why key and ciphertext lengths grow.
- `eta1` and `eta2` configure small-noise sampling.
- `du` and `dv` configure ciphertext compression widths.
- The shared secret is 32 bytes for every ML-KEM parameter set.

## ML-DSA Parameters

FIPS 204 Table 1 defines `q`, `zeta`, `d`, `tau`, `lambda`, `gamma1`, `gamma2`, `(k, l)`, `eta`, `beta`, `omega`, challenge entropy, expected repetitions, and claimed security category. FIPS 204 Table 2 defines key and signature sizes.

| Parameter set | n | q | zeta | d | tau | lambda | gamma1 | gamma2 | k | l | eta | beta | omega | challenge entropy | Category | pk bytes | sk bytes | sig bytes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| ML-DSA-44 | 256 | 8380417 | 1753 | 13 | 39 | 128 | 2^17 | (q - 1) / 88 | 4 | 4 | 2 | 78 | 80 | 192 | 2 | 1312 | 2560 | 2420 |
| ML-DSA-65 | 256 | 8380417 | 1753 | 13 | 49 | 192 | 2^19 | (q - 1) / 32 | 6 | 5 | 4 | 196 | 55 | 225 | 3 | 1952 | 4032 | 3309 |
| ML-DSA-87 | 256 | 8380417 | 1753 | 13 | 60 | 256 | 2^19 | (q - 1) / 32 | 8 | 7 | 2 | 120 | 75 | 257 | 5 | 2592 | 4896 | 4627 |

Teaching notes:

- The names 44, 65, and 87 reflect `(k, l)` dimensions.
- `q = 8380417` is the modulus for ML-DSA coefficients.
- `zeta = 1753` is a root used by NTT routines.
- `tau` is the number of nonzero coefficients in the challenge polynomial.
- `gamma1`, `gamma2`, `beta`, and `omega` are signing and verification bounds.
- Public key, secret key, and signature sizes are exact byte-level contract values.

## TDD Checklist

- Assert every standardized parameter-set name exactly.
- Assert every dimension and bound from the FIPS table.
- Assert every public key, secret key, ciphertext, shared secret, or signature length.
- Assert ML-KEM and ML-DSA parameter enums are distinct types.
- Keep a shared fixture format for later Rust and vector-loader work.

## Shared Fixture

The fixture at `fixtures/parameter-sets.json` mirrors the values above. It is not an official NIST vector file. It is a repository fixture for cross-language tests and future vector-loader plumbing.

