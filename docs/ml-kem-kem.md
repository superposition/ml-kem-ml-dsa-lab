# ML-KEM KEM Derivation

This document derives the ML-KEM key generation, encapsulation, and decapsulation boundary.

## Source Pointers

| Scheme | FIPS source | Algorithm source | Scope in this ticket |
| --- | --- | --- | --- |
| ML-KEM | FIPS 203 | Section 6, Algorithms 16-18 | ML-KEM-512 internal KEM test-hook path |
| ML-KEM | FIPS 203 | Section 7, Algorithms 19-21 | Public API contract and fail-closed boundary |

## Public API Contract

The public C++ surface is:

```text
ml_kem_keygen(parameter_set) -> MlKemKeyPair
ml_kem_encapsulate(parameter_set, public_key) -> (MlKemCiphertext, SharedSecret)
ml_kem_decapsulate(parameter_set, secret_key, ciphertext) -> SharedSecret
```

The byte lengths are selected by the parameter set:

| Parameter set | Encapsulation key | Decapsulation key | Ciphertext | Shared secret |
| --- | ---: | ---: | ---: | ---: |
| ML-KEM-512 | 800 | 1632 | 768 | 32 |
| ML-KEM-768 | 1184 | 2400 | 1088 | 32 |
| ML-KEM-1024 | 1568 | 3168 | 1568 | 32 |

Production public functions still fail closed with `NotImplemented`. `ml_kem_encapsulate`
checks the public-key length before throwing, and `ml_kem_decapsulate` checks both the
decapsulation-key and ciphertext lengths before throwing.

The stubs must not be replaced until official ACVP/KAT vector ingestion and real
SHA3/SHAKE plumbing exist. The repository-owned fixture for this ticket is deterministic
derivation data, not a NIST vector file.

## Internal Key Generation

FIPS `ML-KEM.KeyGen_internal(d, z)` takes two 32-byte seeds:

- `d` feeds K-PKE key generation,
- `z` is stored for implicit rejection during decapsulation.

For ML-KEM-512:

```text
(ekPKE, dkPKE) = K-PKE.KeyGen(d)
ek = ekPKE
dk = dkPKE || ek || H(ek) || z
```

The resulting decapsulation key length is:

```text
768 + 800 + 32 + 32 = 1632 bytes
```

The C++ test path implements this as `ml_kem_keygen_internal_512_test`.

## Internal Encapsulation

FIPS `ML-KEM.Encaps_internal(ek, m)` takes an encapsulation key and a 32-byte random
message seed:

```text
(K, r) = G(m || H(ek))
c = K-PKE.Encrypt(ek, m, r)
return (K, c)
```

The C++ test path implements this as `ml_kem_encaps_internal_512_test`. It rejects malformed
ML-KEM-512 encapsulation-key lengths, derives deterministic test-hook `K` and `r`, and returns
a 768-byte ciphertext plus a 32-byte shared secret.

## Internal Decapsulation

FIPS `ML-KEM.Decaps_internal(dk, c)` parses:

```text
dkPKE = dk[0 : 768]
ekPKE = dk[768 : 1568]
h = dk[1568 : 1600]
z = dk[1600 : 1632]
```

It decrypts, recomputes the encapsulation path, and performs implicit rejection:

```text
m' = K-PKE.Decrypt(dkPKE, c)
(K', r') = G(m' || h)
Kbar = J(z || c)
c' = K-PKE.Encrypt(ekPKE, m', r')
return K' if c == c' else Kbar
```

The C++ test path implements this as `ml_kem_decaps_internal_512_test`. The ciphertext
comparison is accumulated without early exit, and the shared-secret selection is mask-based so
the code shape matches the constant-time review target. This is still an internal review status,
not a production constant-time claim.

## Error Classes

Current public errors:

- malformed encapsulation key: `std::invalid_argument`,
- malformed decapsulation key: `std::invalid_argument`,
- malformed ciphertext: `std::invalid_argument`,
- algorithm not enabled for production: `pqcore::NotImplemented`.

Current internal test-hook errors:

- malformed ML-KEM-512 encapsulation key: `std::invalid_argument`,
- malformed ML-KEM-512 decapsulation key: `std::invalid_argument`,
- malformed ML-KEM-512 ciphertext: `std::invalid_argument`,
- deterministic hook called in a normal build: `std::logic_error`.

Secret-dependent decapsulation failure does not expose a distinct error class. A malformed but
well-sized ciphertext returns a fallback shared secret.

## Side-Channel Review

The current side-channel review state is recorded in `docs/side-channel-review.md` and
`audits/side-channel/manifest.json`.

The internal ML-KEM-512 decapsulation failure path is reviewed for the deterministic test hook:
ciphertext equality is accumulated across the full ciphertext, and shared-secret selection is
mask-based. This is still a production blocker until real hashing, official vectors, optimized
compiler-output review, and external cryptographic review land.

## Deterministic Test Mode

FIPS 203 defines `H`, `G`, and `J` through SHA3/SHAKE functions. This repository does not yet
implement SHA3/SHAKE, so the C++ implementation exposes only deterministic `_test` helpers
behind `PQCORE_ENABLE_TEST_SAMPLING`.

Normal builds compile release-policy stubs for the internal helpers. They throw if called.

## Official Vectors

Official vector ingestion is planned under the ACVP/KAT vector gate. Once real vectors are
available, public tests should cover:

- `ML-KEM.KeyGen_internal`,
- `ML-KEM.Encaps_internal`,
- `ML-KEM.Decaps_internal`,
- public `ML-KEM.KeyGen`,
- public `ML-KEM.Encaps`,
- public `ML-KEM.Decaps`.

Only after those vector tests pass should public `ml_kem_keygen`, `ml_kem_encapsulate`, and
`ml_kem_decapsulate` stop failing closed.

## Fixture

The fixture at `fixtures/ml-kem-kem-examples.json` records ML-KEM-512 internal KEM dimensions,
decapsulation-key layout, deterministic test-hook prefixes, and the production fail-closed
boundary. It is not an official NIST vector file.

## Readiness Caveat

Passing these internal KEM tests does not prove production ML-KEM correctness. Production
readiness still requires official vectors, real SHA3/SHAKE/XOF/PRF plumbing, entropy review,
constant-time review, and external cryptographic review.
