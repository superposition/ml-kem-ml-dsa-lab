# ML-KEM Internal PKE Derivation

This document derives the first internal public-key encryption layer used by ML-KEM.

## Source Pointers

| Scheme | FIPS source | PKE source | Scope in this ticket |
| --- | --- | --- | --- |
| ML-KEM | FIPS 203 | Section 5, Algorithms 13-15 | ML-KEM-512 C++ internal K-PKE test-hook path |

## Layer Boundary

K-PKE is not the public ML-KEM API. ML-KEM wraps K-PKE with hashing, key confirmation, and implicit-rejection behavior. The public `ml_kem_keygen`, `ml_kem_encapsulate`, and `ml_kem_decapsulate` functions still fail closed.

This ticket implements the ML-KEM-512 internal layer first. ML-KEM-768 and ML-KEM-1024 remain future expansions.

## Dimensions

For ML-KEM-512:

- `k = 2`,
- `eta1 = 3`,
- `eta2 = 2`,
- `du = 10`,
- `dv = 4`,
- encryption key length is `384 * k + 32 = 800` bytes,
- decryption key length is `384 * k = 768` bytes,
- ciphertext length is `32 * (du * k + dv) = 768` bytes.

The matrix `A` has shape `2 x 2`. Secret, error, and ciphertext vector components have length `2`.

## Hash/XOF/PRF Boundary

FIPS 203 uses `G`, `XOF`, and `PRF` based on SHA3/SHAKE functions. The C++ implementation now has production-named helpers for:

- `G(seed)` through SHA3-512 seed expansion,
- `XOF(rho || j || i)` through SHAKE128,
- `PRF_eta(seed, nonce)` through SHAKE256.

The full K-PKE algorithm path still remains an internal derivation surface. The deterministic test hook preserves the algorithm shape for derivation tests, but it is not cryptographic. Normal builds keep test-only helpers behind release-policy stubs that throw.

## Key Generation

K-PKE key generation expands a 32-byte seed into two 32-byte seeds:

- `rho` generates matrix `A`,
- `sigma` generates secret vector `s` and error vector `e`.

For each row `i` and column `j`:

```text
A[i, j] = SampleNTT(rho || j || i)
```

Then:

```text
s[i] = SamplePolyCBD_eta1(PRF_eta1(sigma, nonce))
e[i] = SamplePolyCBD_eta1(PRF_eta1(sigma, nonce))
t_hat = A * NTT(s) + NTT(e)
```

The encryption key is `ByteEncode_12(t_hat) || rho`. The decryption key is `ByteEncode_12(NTT(s))`.

## Encryption

K-PKE encryption decodes `t_hat` and `rho` from the encryption key, regenerates matrix `A`, samples:

- `y` with `eta1`,
- `e1` with `eta2`,
- `e2` with `eta2`.

It computes:

```text
u = InverseNTT(A^T * NTT(y)) + e1
v = InverseNTT(t_hat^T * NTT(y)) + e2 + Decompress_1(ByteDecode_1(m))
```

The ciphertext is:

```text
ByteEncode_du(Compress_du(u)) || ByteEncode_dv(Compress_dv(v))
```

## Decryption

K-PKE decryption decodes and decompresses `u` and `v`, decodes `s_hat` from the decryption key, and computes:

```text
w = v - InverseNTT(s_hat^T * NTT(u))
m = ByteEncode_1(Compress_1(w))
```

## Secret-Bearing Values

The following are secret-bearing and require constant-time review before production use:

- key generation seed,
- `sigma`,
- `s`,
- `e`,
- encryption randomness,
- `y`,
- `e1`,
- `e2`,
- decryption key,
- decoded secret NTT vector.

## Fixture

The fixture at `fixtures/ml-kem-pke-examples.json` records ML-KEM-512 dimensions and deterministic test-hook prefixes. It is not an official NIST vector file.

## Readiness Caveat

Passing internal K-PKE tests does not prove public ML-KEM correctness. The outer KEM transform, official vector execution, entropy review, and constant-time review are still required.
