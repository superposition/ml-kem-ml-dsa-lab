# ML-DSA Rounding, Hints, and Challenge Derivation

This document derives the helper routines used by ML-DSA signing and verification before the
public signature API is implemented.

## Source Pointers

| Scheme | FIPS source | Algorithm source | Scope in this ticket |
| --- | --- | --- | --- |
| ML-DSA | FIPS 204 | Section 7.4, Algorithms 35-40 | Rounding, decomposition, high/low bits, hints |
| ML-DSA | FIPS 204 | Section 7.5, Algorithm 29 | Challenge polynomial construction through `SampleInBall` |

## Parameters

The helper routines use the selected ML-DSA parameter set:

| Parameter set | `q` | `d` | `tau` | `lambda` | `gamma2` | `alpha = 2 * gamma2` | `(q - 1) / alpha` |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| ML-DSA-44 | 8380417 | 13 | 39 | 128 | 95232 | 190464 | 44 |
| ML-DSA-65 | 8380417 | 13 | 49 | 192 | 261888 | 523776 | 16 |
| ML-DSA-87 | 8380417 | 13 | 60 | 256 | 261888 | 523776 | 16 |

`Power2Round` uses `2^d = 8192`. `Decompose`, `HighBits`, `LowBits`, `MakeHint`,
and `UseHint` use `alpha = 2 * gamma2`.

## Centered Remainders

The FIPS `mod+-` operation returns a centered remainder. For an even modulus `m`, this
repository uses the interval:

```text
[-m / 2 + 1, m / 2]
```

For example, with `m = 8192`:

```text
4096 mod+- 8192 = 4096
4097 mod+- 8192 = -4095
```

This exact boundary matters because it decides whether the rounded-off part is positive or
negative.

## Power2Round

`Power2Round(r)` decomposes `r mod q` into high and low parts:

```text
r = r1 * 2^d + r0 mod q
```

Tiny example with base `4`:

```text
r = 11
r0 = 11 mod+- 4 = -1
r1 = (11 - (-1)) / 4 = 3
3 * 4 - 1 = 11
```

Real ML-DSA example:

```text
r = 4097
r0 = -4095
r1 = 1
1 * 8192 - 4095 = 4097
```

`Power2Round` is used for public-key compression. The high part is stored, while the low part
is retained in the secret key.

## Decompose, HighBits, and LowBits

`Decompose(r)` uses `alpha = 2 * gamma2`:

```text
r = r1 * alpha + r0 mod q
```

It differs from `Power2Round` at the wrap boundary. Because `alpha` divides `q - 1`, a direct
decomposition of `q - 1` would produce the last high-bit bucket and low part `0`. FIPS instead
returns:

```text
Decompose(q - 1) = (0, -1)
```

Tiny example with `q = 17` and `alpha = 4`:

```text
Decompose(16) = (0, -1)
0 * 4 - 1 = -1 = 16 mod 17
```

`HighBits(r)` returns the high part of `Decompose(r)`. `LowBits(r)` returns the low part.
The C++ implementation exposes all three routines through parameterized helpers.

## Hints

Hints exist because verification needs to reconstruct the signer's rounded commitment without
learning the dropped low-order information directly. `MakeHint(z, r)` reports whether adding
`z` changes the high bits of `r`:

```text
MakeHint(z, r) = HighBits(r) != HighBits(r + z)
```

Example at the real ML-DSA-44 boundary:

```text
gamma2 = 95232
HighBits(gamma2) = 0
HighBits(gamma2 + 1) = 1
MakeHint(1, gamma2) = true
```

`UseHint(h, r)` adjusts the high bits only when the hint is set. If the low part is positive,
the high part increments modulo `(q - 1) / (2 * gamma2)`. If the low part is zero or negative,
the high part decrements modulo that same value.

For ML-DSA-44, `(q - 1) / (2 * gamma2) = 44`, so:

```text
UseHint(true, 0) = 43
UseHint(true, gamma2) = 1
UseHint(false, gamma2) = HighBits(gamma2) = 0
```

## Challenge Polynomial

FIPS `SampleInBall(rho)` constructs the challenge polynomial `c`. The output must have:

- exactly `tau` nonzero coefficients,
- every coefficient in `{-1, 0, 1}`.

The FIPS input seed length is `lambda / 4` bytes:

| Parameter set | `lambda / 4` bytes | `tau` |
| --- | ---: | ---: |
| ML-DSA-44 | 32 | 39 |
| ML-DSA-65 | 48 | 49 |
| ML-DSA-87 | 64 | 60 |

This repository already has a raw byte-stream `SampleInBall` derivation. This ticket adds
`ml_dsa_challenge_polynomial_from_bytes`, which applies the selected parameter set's `tau` and
lets tests exercise the challenge polynomial construction without claiming a production XOF.

## Secret-Bearing Values

These helpers are correctness-critical. In full signing and verification, the following values
can be secret-bearing or derived from secret material:

- low bits from secret-dependent computations,
- hint inputs,
- rejection-sampling bounds,
- challenge-dependent products.

The helper tests prove algebraic invariants, not constant-time behavior.

## Fixture

The fixture at `fixtures/ml-dsa-helper-examples.json` records parameterized bounds and
repository-owned examples for rounding, hints, and challenge construction. It is not an official
NIST vector file.

## Readiness Caveat

Passing these helper tests does not prove signature correctness or security. Production ML-DSA
still requires key generation, signing, verification, official vectors, constant-time review, and
external cryptographic review.
