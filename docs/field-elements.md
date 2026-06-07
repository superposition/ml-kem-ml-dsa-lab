# Field Element Derivation

This document derives the finite-field element abstraction used by ML-KEM and ML-DSA.

## Source Pointers

| Scheme | FIPS source | Modulus source | Related operations |
| --- | --- | --- | --- |
| ML-KEM | FIPS 203 | Table 2 gives `q = 3329` | Sections 2.4, 4.2.1, and 4.3 use coefficient arithmetic modulo `q` |
| ML-DSA | FIPS 204 | Table 1 gives `q = 8380417` | Sections 7.4, 7.5, and 7.6 use coefficient arithmetic modulo `q` |

## Mathematical Object

`Z_q` means integers modulo `q`. Many integers represent the same field element:

```text
0 == q == -q mod q
-1 == q - 1 mod q
```

The implementation stores one canonical representative in `[0, q)`.

## Canonical Representation

Canonical representation is the stored value. For any integer input `x`, normalization computes the equivalent value in `[0, q)`.

Examples:

```text
normalize(q) = 0
normalize(q + 1) = 1
normalize(-1) = q - 1
normalize(-q - 1) = q - 1
```

Test invariant: constructing a field element from any integer used in tests yields a value in `[0, q)`.

## Centered Representation

Centered representation is a view, not a different stored value. It maps high canonical values into negative integers so small coefficients can be reasoned about around zero.

For odd `q`, this implementation maps:

```text
0 ... floor(q / 2)       -> same value
floor(q / 2) + 1 ... q-1 -> value - q
```

Examples:

```text
ModQ<17>(8).centered() = 8
ModQ<17>(9).centered() = -8
```

Test invariant: centered conversion never changes the stored canonical value.

## Arithmetic Closure

Addition, subtraction, negation, and multiplication return new field elements modulo `q`.

Test invariants:

- `q` normalizes to zero.
- `-1` normalizes to `q - 1`.
- addition wraps above `q`.
- subtraction wraps below zero.
- multiplication uses a wide enough intermediate for ML-DSA values.

## Wide Multiplication

ML-DSA uses `q = 8380417`. A product of two canonical representatives can exceed 32-bit signed integer range:

```text
(q - 1) * (q - 1)
```

The C++ implementation therefore multiplies with a 64-bit temporary before reducing modulo `q`.

Test invariant: `(q - 1) * (q - 1) = 1 mod q` for the ML-DSA modulus.

## Constant-Time Caveat

The current `ModQ` type is a first-principles arithmetic representation. It is not a secret wrapper and does not by itself prove constant-time behavior.

Later secret-bearing paths need review for:

- secret-dependent branches,
- secret-dependent memory access,
- debug or stream output of secret coefficients,
- compiler behavior in optimized builds.

The field tests in this issue prove representation and arithmetic invariants, not side-channel safety.

