# Glossary

This glossary is the shared vocabulary for deriving ML-KEM and ML-DSA. It is not a replacement for FIPS 203 or FIPS 204. It is the translation layer that connects standard notation to testable implementation objects.

## Source Hierarchy

1. FIPS 203 defines ML-KEM.
2. FIPS 204 defines ML-DSA.
3. NIST ACVP material is used for vector and validation workflow planning.
4. Repository derivation notes explain how the standard becomes code.

Before implementing algorithm details, re-check the official NIST pages and their errata notes.

## Scheme Names

### `ML-KEM`

Module-Lattice-Based Key-Encapsulation Mechanism, standardized in FIPS 203. Kyber is the legacy family name, but public APIs and docs in this repository should use ML-KEM.

Testable invariant: ML-KEM parameter-set names are exactly `ML-KEM-512`, `ML-KEM-768`, and `ML-KEM-1024`.

### `ML-DSA`

Module-Lattice-Based Digital Signature Algorithm, standardized in FIPS 204. Dilithium is the legacy family name, but public APIs and docs in this repository should use ML-DSA.

Testable invariant: ML-DSA parameter-set names are exactly `ML-DSA-44`, `ML-DSA-65`, and `ML-DSA-87`.

### `Kyber`

Legacy/common name for the algorithm family standardized as ML-KEM. Do not assume an older Kyber draft is byte-compatible with final FIPS 203 ML-KEM.

Testable invariant: compatibility tests must identify the exact standard or draft revision they target.

### `Dilithium`

Legacy/common name for the algorithm family standardized as ML-DSA. Do not use Dilithium as the public API name unless documenting history or compatibility.

Testable invariant: public API names should use ML-DSA terminology.

## Mathematical Objects

### `q`

The modulus for coefficient arithmetic. ML-KEM uses `q = 3329`; ML-DSA uses `q = 8380417`.

Representation: a field element type stores a canonical integer in `[0, q)`.

Testable invariant: any integer input normalizes into `[0, q)`, and `q` normalizes to zero.

### `Z_q`

The integers modulo `q`. Values that differ by a multiple of `q` represent the same element.

Representation: C++ uses `ModQ<Q>`; future Rust should use a type that cannot be confused with a raw integer.

Testable invariant: addition, subtraction, and multiplication remain inside `[0, q)`.

### `R_q`

The polynomial ring `Z_q[x] / (x^256 + 1)`. Each element is a polynomial with 256 coefficients modulo `q`.

Representation: a fixed-size polynomial over field elements.

Testable invariant: multiplication is negacyclic, so `x^256 = -1`.

### `coefficient`

One field element inside a polynomial.

Representation: one canonical modulo-`q` value.

Testable invariant: coefficients are normalized at construction and after arithmetic.

### `polynomial`

A fixed-length list of coefficients interpreted as an element of `R_q`.

Representation: an array of 256 field elements for real scheme code; smaller sizes may be used only in teaching tests.

Testable invariant: zero polynomial has all zero coefficients; multiplication by one returns the original polynomial.

### `vector`

A fixed-length list of polynomials. In module-lattice schemes, vectors are module elements over `R_q`.

Representation: an array of `k` or `l` polynomials, depending on the scheme and operation.

Testable invariant: vector length must match the selected parameter set.

### `matrix`

A rectangular grid of polynomials. Matrix-vector multiplication over `R_q` is a core operation in both schemes.

Representation: a fixed-size two-dimensional array or generated view of polynomials.

Testable invariant: matrix dimensions must match the parameter set and operation.

## Parameters

### `k`

A module dimension. In ML-KEM, `k` selects the size of vectors and matrices. In ML-DSA, `k` is one of the matrix dimensions.

Representation: part of the parameter table.

Testable invariant: byte lengths and vector dimensions agree with the selected `k`.

### `l`

An ML-DSA module dimension used with `k` for matrix and vector sizes.

Representation: part of the ML-DSA parameter table.

Testable invariant: ML-DSA secret vectors and matrix dimensions agree with `l`.

### `eta`

A bound or distribution parameter used for small secret/noise sampling. ML-KEM distinguishes `eta1` and `eta2`; ML-DSA has an `eta` parameter.

Representation: part of the parameter table and sampler configuration.

Testable invariant: sampler output stays within the required bound.

### `du`

ML-KEM compression width for one ciphertext component.

Representation: part of the ML-KEM parameter table.

Testable invariant: encoded ciphertext length changes with `du` as specified by the parameter set.

### `dv`

ML-KEM compression width for the other ciphertext component.

Representation: part of the ML-KEM parameter table.

Testable invariant: encoded ciphertext length changes with `dv` as specified by the parameter set.

### `gamma1`

ML-DSA bound used in signing-related sampling and checks.

Representation: part of the ML-DSA parameter table.

Testable invariant: generated or computed values satisfy the required bound before accepting a signature.

### `gamma2`

ML-DSA decomposition bound used by high/low-bit and hint logic.

Representation: part of the ML-DSA parameter table.

Testable invariant: decomposition and hint tests recombine according to the documented rule.

### `tau`

ML-DSA challenge weight. The challenge polynomial has exactly `tau` nonzero coefficients.

Representation: part of the ML-DSA parameter table.

Testable invariant: challenge generation produces exactly `tau` coefficients in `{-1, 1}` and all other coefficients are zero.

### `omega`

ML-DSA bound on the number of hint positions.

Representation: part of the ML-DSA parameter table and signature checks.

Testable invariant: signatures with too many hints are rejected.

## Byte and Randomness Terms

### `seed`

A byte string used to deterministically expand structured objects such as matrices, secret vectors, masks, or challenges.

Representation: fixed-size byte arrays with domain separation where the standard requires it.

Testable invariant: the same seed and domain inputs produce the same object.

### `XOF`

Extendable-output function. It expands input bytes into as many pseudorandom bytes as an algorithm needs.

Representation: a named primitive behind sampling or matrix generation.

Testable invariant: fixed input produces fixed output for vector tests.

### `PRF`

Pseudorandom function. It deterministically expands secret or seed material into pseudorandom output.

Representation: a named primitive with explicit inputs, nonces, and output length.

Testable invariant: nonce or domain changes change the output.

### `sampler`

A procedure that maps bytes into mathematical objects with a required distribution.

Representation: module-specific functions for rejection sampling, small bounded coefficients, masks, or challenges.

Testable invariant: sampler output stays inside the required domain and is deterministic under test fixtures.

### `encoding`

Mapping a mathematical object into bytes.

Representation: byte-packing functions with exact lengths from the parameter table.

Testable invariant: encoded output length is exact.

### `decoding`

Mapping bytes back into a mathematical object.

Representation: parser functions with explicit length and validity checks.

Testable invariant: malformed lengths are rejected and valid round trips preserve the represented object.

### `compression`

ML-KEM operation that maps coefficients to fewer bits for compact ciphertexts.

Representation: parameterized by compression widths such as `du` and `dv`.

Testable invariant: output uses the expected number of bits; decompression is approximate, not necessarily exact.

### `decompression`

ML-KEM operation that maps compressed values back into field elements.

Representation: inverse-style approximation of compression.

Testable invariant: decompressed values are in `[0, q)` and satisfy the expected error bound.

## Computational Domains

### `coefficient domain`

The ordinary polynomial representation as coefficients in `R_q`.

Representation: arrays of field elements.

Testable invariant: schoolbook multiplication operates in this domain.

### `NTT domain`

A transformed polynomial representation used for faster multiplication.

Representation: a distinct type or explicit marker should prevent accidental mixing with coefficient-domain values.

Testable invariant: forward NTT followed by inverse NTT returns the original polynomial.

### `public input`

Data that an attacker may know or choose: public keys, ciphertexts, signatures, messages, contexts, and many seeds.

Representation: byte arrays or parsed public structures.

Testable invariant: malformed public input is rejected without reading out of bounds.

### `secret-bearing input`

Data that contains or is derived from private material: secret keys, secret vectors, signing masks, decapsulation fallback secrets, and some PRF inputs.

Representation: types and APIs that avoid accidental debug output and are marked for constant-time review.

Testable invariant: secret-bearing types do not print raw bytes by default.

## Repository Modules

### `params`

Parameter tables for names, dimensions, bounds, compression widths, and byte lengths.

Testable invariant: every selected parameter set exposes exact standardized names and byte lengths.

### `field`

Modulo-`q` arithmetic for coefficients.

Testable invariant: normalization and arithmetic stay in `[0, q)`.

### `polynomial`

Arithmetic in `R_q`.

Testable invariant: negacyclic multiplication implements `x^256 = -1`.

### `encoding`

Byte packing and unpacking.

Testable invariant: round trips and exact lengths match the selected parameter set.

### `sampling`

Seed expansion and sampler logic.

Testable invariant: sampler output stays within the required domain.

### `ntt`

Forward and inverse number theoretic transform.

Testable invariant: NTT multiplication agrees with schoolbook multiplication.

### `pke`

ML-KEM internal public-key encryption layer.

Testable invariant: fixed-seed internal encryption fixtures are deterministic.

### `kem`

ML-KEM public key generation, encapsulation, and decapsulation.

Testable invariant: official vectors pass for each parameter set before fail-closed stubs are removed.

### `rounding`

ML-DSA helper routines such as `Power2Round`, `Decompose`, `HighBits`, `LowBits`, `MakeHint`, and `UseHint`.

Testable invariant: helper routines recombine or adjust values according to the standard.

### `dsa`

ML-DSA public key generation, signing, and verification.

Testable invariant: official vectors pass before fail-closed stubs are removed.

