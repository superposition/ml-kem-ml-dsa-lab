# Polynomial Ring Derivation

This document derives polynomial arithmetic for ML-KEM and ML-DSA.

## Source Pointers

| Scheme | FIPS source | Ring source | Related operations |
| --- | --- | --- | --- |
| ML-KEM | FIPS 203 | Section 2.4 defines the polynomial ring over `Z_q` | Key generation, internal public-key encryption, NTT, and encoding use polynomials |
| ML-DSA | FIPS 204 | Section 2.3 defines polynomial arithmetic over `R_q` | Key generation, signing, verification, NTT, decomposition, and hints use polynomials |

Both standards use `n = 256`. ML-KEM uses `q = 3329`; ML-DSA uses `q = 8380417`.

## Mathematical Object

`R_q = Z_q[x] / (x^256 + 1)` means a polynomial has exactly 256 coefficients, and each coefficient is a field element modulo `q`.

The quotient by `x^256 + 1` makes:

```text
x^256 = -1
```

So any multiplication term with degree 256 or higher wraps around with a sign flip.

## Teaching Ring

The C++ tests also use a tiny ring:

```text
Z_17[x] / (x^4 + 1)
```

This keeps the wraparound rule visible. In that ring:

```text
x^3 * x = x^4 = -1
```

So the product is the constant polynomial `16` modulo `17`.

## Representation

The implementation stores a fixed-size array of field elements:

```text
Polynomial<N, Q> = [ModQ<Q>; N]
```

Real scheme aliases are:

```text
MlKemPolynomial = Polynomial<256, 3329>
MlDsaPolynomial = Polynomial<256, 8380417>
```

Smaller rings are allowed only for derivation and tests.

## Construction

Every coefficient is normalized through the field element type. Raw input values such as `q`, `-1`, and `q + 1` are accepted only at construction boundaries and immediately reduced into `[0, q)`.

Test invariants:

- zero polynomial has all coefficients equal to zero,
- construction normalizes every coefficient,
- ML-KEM and ML-DSA polynomial aliases are distinct types.

## Addition and Subtraction

Addition and subtraction are coefficient-wise in `Z_q`.

Test invariants:

- addition preserves each coefficient position,
- subtraction wraps below zero through the field type,
- all resulting coefficients remain canonical.

## Negacyclic Multiplication

Schoolbook multiplication is used as the derivation oracle:

1. Multiply every pair of coefficients.
2. If `i + j < N`, add the term to coefficient `i + j`.
3. If `i + j >= N`, subtract the term from coefficient `i + j - N`.

The subtraction in step 3 comes from `x^N = -1`.

Test invariants:

- `x^(N - 1) * x = -1`,
- multiplication by zero returns zero,
- multiplication by one returns the original polynomial,
- distribution over addition holds for the tested examples.

## Why Keep Schoolbook Multiplication

NTT exists to make multiplication fast. Schoolbook multiplication stays useful because it is direct, easy to inspect, and a strong correctness oracle for future NTT tests.

Future NTT work should prove that an NTT-domain product decodes to the same coefficients as this schoolbook path for shared fixtures.

## Shared Fixture

The fixture at `fixtures/polynomial-examples.json` records tiny-ring and real-ring examples for future cross-language tests. It is not an official NIST vector file.

## Constant-Time Caveat

This polynomial type proves algebraic representation and arithmetic invariants. It does not prove constant-time behavior.

Later secret-bearing paths need review for:

- secret-dependent indexing,
- data-dependent branches,
- debug or stream output of secret coefficients,
- optimized builds that transform arithmetic unexpectedly.

