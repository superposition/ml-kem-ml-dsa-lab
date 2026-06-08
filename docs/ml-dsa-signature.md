# ML-DSA Key Generation, Signing, and Verification Derivation

This document derives the public ML-DSA signature API contract and records the internal
signing-flow boundary used for tests.

## Source Pointers

| Scheme | FIPS source | Algorithm source | Scope in this ticket |
| --- | --- | --- | --- |
| ML-DSA | FIPS 204 | Section 5, Algorithms 1-3 | Public key generation, signing, and verification contract |
| ML-DSA | FIPS 204 | Section 6, Algorithms 6-8 | Internal key generation, signing, and verification flow |
| ML-DSA | FIPS 204 | Section 7.2, Algorithms 22-28 | Key and signature byte-layout contract |

## Public API Contract

The public C++ surface is:

```text
ml_dsa_keygen(parameter_set) -> MlDsaKeyPair
ml_dsa_sign(parameter_set, secret_key, message, context) -> Signature
ml_dsa_verify(parameter_set, public_key, message, signature, context) -> bool
```

The byte lengths are selected by the parameter set:

| Parameter set | Public key | Secret key | Signature |
| --- | ---: | ---: | ---: |
| ML-DSA-44 | 1312 | 2560 | 2420 |
| ML-DSA-65 | 1952 | 4032 | 3309 |
| ML-DSA-87 | 2592 | 4896 | 4627 |

Production public functions still fail closed with `NotImplemented`. `ml_dsa_sign` checks secret-key
length and context length before throwing. `ml_dsa_verify` checks public-key length, signature length,
and context length before throwing.

The public stubs must not be replaced until official vector execution, ML-DSA encoding, ML-DSA NTT,
final expansion plumbing, and signature-specific constant-time review exist.

## Message Formatting

FIPS `ML-DSA.Sign` and `ML-DSA.Verify` format the message as:

```text
M' = BytesToBits(IntegerToBytes(0, 1) || IntegerToBytes(|ctx|, 1) || ctx) || M
```

The context is a byte string with at most 255 bytes. The C++ derivation exposes a byte-level
`ml_dsa_format_message` helper:

```text
formatted = 0x00 || uint8(context_length) || context || message
```

This is not a substitute for the final bit-level API; it is the repository's current byte-oriented
test boundary.

## Key Layout

FIPS `pkEncode` stores:

```text
pk = rho || t1
```

where `rho` is 32 bytes and `t1` is the compressed public vector.

FIPS `skEncode` stores:

```text
sk = rho || K || tr || s1 || s2 || t0
```

where:

- `rho` is the public matrix seed,
- `K` is a private signing seed,
- `tr = H(pk, 64)`,
- `s1`, `s2`, and `t0` are encoded polynomial vectors.

The deterministic internal test hook preserves the header shape:

```text
sk[0:32] = rho
sk[32:64] = K
sk[64:128] = tr
```

The remaining bytes are repository-owned deterministic fixture payload, not FIPS-conformant
polynomial encodings.

## Signature Layout

FIPS `sigEncode` stores:

```text
signature = c_tilde || z || h
```

The lengths are:

| Parameter set | `c_tilde` | `z` bytes | `h` bytes | Total |
| --- | ---: | ---: | ---: | ---: |
| ML-DSA-44 | 32 | 2304 | 84 | 2420 |
| ML-DSA-65 | 48 | 3200 | 61 | 3309 |
| ML-DSA-87 | 64 | 4480 | 83 | 4627 |

The deterministic internal test hook keeps this same outer layout. It treats the `z || h` region as a
test witness so verification can recompute and compare `c_tilde` without claiming full lattice
correctness.

## Internal Key Generation

FIPS `ML-DSA.KeyGen_internal(xi)` expands a 32-byte seed into:

- `rho`, which derives the public matrix,
- `rho_prime`, which derives secret vectors,
- `K`, which is used during signing.

It computes:

```text
t = A * s1 + s2
(t1, t0) = Power2Round(t)
pk = pkEncode(rho, t1)
tr = H(pk, 64)
sk = skEncode(rho, K, tr, s1, s2, t0)
```

The C++ `ml_dsa_keygen_internal_test` mirrors the byte-length and header responsibilities, but it
does not implement `ExpandA`, `ExpandS`, ML-DSA NTT, or key packing.

## Internal Signing

FIPS `ML-DSA.Sign_internal(sk, M', rnd)`:

1. decodes `rho`, `K`, `tr`, `s1`, `s2`, and `t0`,
2. computes `mu = H(tr || M', 64)`,
3. computes signing randomness `rho_second = H(K || rnd || mu, 64)`,
4. repeats the rejection loop until the response and hint checks pass,
5. returns `sigEncode(c_tilde, z, h)`.

The deterministic internal test hook preserves:

- the `tr || M'` message representative boundary,
- the `K || rnd || mu` signing-randomness boundary,
- a deterministic rejection-loop fixture with one rejected attempt,
- signature layout as `c_tilde || witness`,
- hint-count checks against `omega`,
- response-marker checks that exercise verification rejection paths.

This is a flow harness. It is not a valid ML-DSA signature algorithm.

The production-named C++ hash helpers now use SHAKE256 for:

- `tr = H(pk, 64)`,
- `mu = H(tr || M', 64)`,
- `rho_second = H(K || rnd || mu, 64)`,
- `c_tilde = H(mu || witness, lambda / 4)`.

They define the hash boundary for later vector execution. They do not complete `ExpandA`,
`ExpandS`, `ExpandMask`, ML-DSA NTT, key packing, or the production rejection loop.

## Internal Verification

FIPS `ML-DSA.Verify_internal(pk, M', signature)`:

1. decodes `rho` and `t1`,
2. decodes `c_tilde`, `z`, and `h`,
3. recomputes `tr = H(pk, 64)` and `mu = H(tr || M', 64)`,
4. reconstructs the commitment with the public relation and `UseHint`,
5. rejects if bounds or hint checks fail,
6. accepts only if recomputed `c_tilde` matches the stored value.

The deterministic internal test hook returns `false` for malformed public-key or signature lengths,
modified messages, modified public keys, modified signatures, response-bound failures, and hint-count
failures.

## Deterministic Versus Hedged Signing

FIPS public signing uses fresh 32-byte randomness for the default hedged variant. The optional
deterministic variant uses 32 zero bytes. The C++ internal test hook takes explicit 32-byte
`signing_random` so tests can exercise the signing flow reproducibly without configuring production
entropy.

## Side-Channel Review

The current side-channel review state is recorded in `docs/side-channel-review.md` and
`audits/side-channel/manifest.json`.

The deterministic rejection loop is reviewed only as a flow harness. It intentionally exercises a
rejected attempt before deterministic acceptance; it is not production signing behavior. Public ML-DSA
signing remains fail-closed until the final rejection loop, hint handling, optimized compiler output,
and external cryptographic review are complete.

## Official Vectors

Official vector files are vendored under the ACVP/KAT vector gate. Once the missing ML-DSA primitives
exist, public tests should cover:

- `ML-DSA.KeyGen_internal`,
- `ML-DSA.Sign_internal`,
- `ML-DSA.Verify_internal`,
- public `ML-DSA.KeyGen`,
- public `ML-DSA.Sign`,
- public `ML-DSA.Verify`,
- modified-message, modified-public-key, and modified-signature negatives.

Only after those vector tests pass should public `ml_dsa_keygen`, `ml_dsa_sign`, and `ml_dsa_verify`
stop failing closed.

## Fixture

The fixture at `fixtures/ml-dsa-signature-examples.json` records parameterized byte layouts,
deterministic internal-test prefixes, public fail-closed behavior, and negative verification cases. It is
not an official NIST vector file.

## Readiness Caveat

Passing these internal signing-flow tests does not prove ML-DSA correctness or security. Production
readiness still requires ML-DSA encoding, ML-DSA NTT, official vector execution, constant-time review,
fuzzing, and external cryptographic review.
