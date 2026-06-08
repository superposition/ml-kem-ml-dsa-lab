# Sampling Derivation

This document derives the first sampling boundaries for ML-KEM and ML-DSA and records how deterministic tests are isolated from production entropy.

## Source Pointers

| Scheme | FIPS source | Sampling source | Scope in this ticket |
| --- | --- | --- | --- |
| SHA-3/SHAKE | FIPS 202 | SHA3-256, SHA3-512, SHAKE128, SHAKE256 | C++ primitive wrappers and known-answer tests |
| ML-KEM | FIPS 203 | Section 4.1; Section 4.2.2, Algorithms 7 and 8; Appendix B Table 4 | C++ SHAKE-backed PRF/XOF wrappers, test-only expansion hook, `SampleNTT` byte rejection, and `SamplePolyCBD` |
| ML-DSA | FIPS 204 | Section 7.3, Algorithms 29-34; Algorithms 14 and 15; Appendix C Table 3 | C++ SHAKE-backed hash boundary, coefficient decoders, bounded sampler, and `SampleInBall` byte-stream derivation |

## Entropy Versus Deterministic Expansion

Entropy is external randomness. Production key generation and randomized signing need reviewed entropy sources and explicit failure modes.

Deterministic expansion starts from seed bytes and domain inputs. It is required for reproducible vectors and for standard algorithms that expand seeds through XOF or PRF functions.

The C++ SHA-3 layer implements SHA3-256, SHA3-512, SHAKE128, and SHAKE256. The C++ `deterministic_test_expand` helper is still only compiled when `PQCORE_ENABLE_TEST_SAMPLING` is defined for tests. It is a fixture generator, not a cryptographic XOF or PRF.

The default production path is fail-closed: `production_random_bytes` throws until a reviewed entropy source is wired.

## ML-KEM PRF And XOF Shape

FIPS 203 defines:

```text
PRF_eta(seed, nonce) = SHAKE256(seed || nonce, 64 * eta bytes)
```

where `eta` is 2 or 3.

FIPS 203 also defines an XOF wrapper over SHAKE128 whose squeeze length is measured in bytes. `SampleNTT` absorbs a 34-byte input: a 32-byte seed plus two index bytes.

The production-named C++ wrappers are:

- `ml_kem_prf(seed, nonce, eta)`,
- `ml_kem_xof(seed_with_indices, output_bytes)`,
- `ml_kem_sample_ntt(seed_with_indices)`.

The C++ test hooks preserve the same input shapes behind the test flag:

- `ml_kem_prf_test(seed, nonce, eta)`,
- `ml_kem_xof_test(seed_with_indices, output_bytes)`.

These hooks are deterministic and domain-separated, but not cryptographic.

## ML-KEM SampleNTT

`SampleNTT` consumes three bytes at a time. Each group produces two 12-bit candidates:

```text
d1 = C[0] + 256 * (C[1] mod 16)
d2 = floor(C[1] / 16) + 16 * C[2]
```

Candidates are accepted only when they are less than `q = 3329`.

Test invariants:

- rejected candidates do not fill output slots,
- every emitted coefficient is in `[0, q)`,
- deterministic seed/index changes change test-hook output.

Appendix B warns that production implementations should avoid bounding this loop if possible. If a limit is used, it must not be lower than the standard's limit, and failure must destroy intermediate results.

## ML-KEM SamplePolyCBD

`SamplePolyCBD_eta` consumes exactly `64 * eta` bytes and converts them to bits in little-endian bit order. For each coefficient:

```text
x = sum eta low bits
y = sum eta following bits
coefficient = x - y mod q
```

Test invariants:

- input length is exact,
- `eta` is only 2 or 3,
- centered coefficients stay in `[-eta, eta]`.

## ML-DSA Sampling Boundaries

FIPS 204 uses several samplers:

- `CoeffFromThreeBytes` rejects values outside `[0, q)`,
- `CoeffFromHalfByte` rejects nibble values that do not map into `[-eta, eta]`,
- `RejBoundedPoly` uses `CoeffFromHalfByte`,
- `SampleInBall` creates exactly `tau` nonzero coefficients in `{ -1, 1 }`.

The C++ code implements the byte-to-coefficient boundaries, byte-stream versions of `RejBoundedPoly`
and `SampleInBall`, and SHAKE256-backed hash boundaries for `tr`, `mu`, signing seed derivation, and
commitment challenge bytes. Full ML-DSA `ExpandA`, `ExpandS`, and `ExpandMask` remain future work.

## Statistical Tests

Bounds and reproducibility tests are necessary but not sufficient. A sampler can satisfy every local bound and still have a biased distribution if it uses the wrong byte expansion or rejection rule.

Production readiness still requires:

- official vectors,
- reviewed SHAKE/XOF/PRF implementation usage,
- reviewed entropy sources,
- side-channel review for secret-bearing samples.

## Fixture

The fixture at `fixtures/sampling-examples.json` records deterministic-hook examples, rejection examples, and sampler bounds for future cross-language comparison. It is not an official NIST vector file.
