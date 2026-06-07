# NTT Derivation

This document derives the Number Theoretic Transform representation used first for ML-KEM.

## Source Pointers

| Scheme | FIPS source | NTT source | Scope in this ticket |
| --- | --- | --- | --- |
| ML-KEM | FIPS 203 | Section 4.3, Algorithms 9-12, Appendix A | C++ forward NTT, inverse NTT, and NTT-domain multiplication |
| ML-DSA | FIPS 204 | Sections 7.5 and 7.6, Algorithms 41-48 | Documentation and future Rust tests only |

## Representation Boundary

The coefficient domain is `R_q = Z_q[x] / (x^256 + 1)`.

The NTT domain is a different representation of the same mathematical object. It must not be treated as a normal coefficient array. In C++, `MlKemPolynomial` and `MlKemNttPolynomial` are distinct types so NTT-domain multiplication cannot silently accept coefficient-domain inputs.

## Root And Ordering

ML-KEM uses `q = 3329` and root `zeta = 17`.

FIPS 203 orders NTT powers by `BitRev7(i)`, the reversal of the seven-bit representation of `i`.

Examples:

```text
BitRev7(0) = 0
BitRev7(1) = 64
BitRev7(2) = 32
zeta^BitRev7(0) mod q = 1
zeta^BitRev7(1) mod q = 1729
zeta^BitRev7(2) mod q = 2580
```

The C++ implementation computes these values from the root rather than storing the Appendix A table by hand. The tests pin representative values from the FIPS table.

## Forward Transform

`ml_kem_ntt` follows the FIPS 203 butterfly structure:

1. Start with coefficient-domain values.
2. Walk block sizes from `128` down to `2`.
3. Use one `zeta^BitRev7(i)` value per block.
4. Replace each pair of halves with butterfly sums and differences.
5. Return an `MlKemNttPolynomial`.

Test invariants:

- the zero polynomial stays zero,
- a constant polynomial has the expected pair ordering,
- inverse after forward returns the original polynomial.

## Inverse Transform

`ml_kem_inverse_ntt` walks the butterfly structure in reverse order, then multiplies every coefficient by `3303`, the inverse of `128` modulo `3329`.

Test invariant:

```text
InverseNTT(NTT(f)) = f
```

for representative polynomials.

## NTT-Domain Multiplication

NTT-domain multiplication is performed by 128 independent base-case multiplications. For each block:

```text
(c0, c1) = (a0*b0 + a1*b1*gamma, a0*b1 + a1*b0)
```

where:

```text
gamma = zeta^(2*BitRev7(i) + 1)
```

The oracle test is:

```text
InverseNTT(NTT(f) * NTT(g)) = schoolbook_negacyclic(f, g)
```

Schoolbook multiplication remains the correctness oracle because it is direct and coefficient-domain.

## ML-DSA Difference

ML-DSA uses a different modulus, root, and NTT specification. FIPS 204 lists `q = 8380417` and `zeta = 1753` in the parameter table and defines its NTT routines separately. This ticket documents that boundary and implements ML-KEM first.

Future ML-DSA work should add separate constants, tests, and domain types rather than reusing ML-KEM assumptions.

## Shared Fixture

The fixture at `fixtures/ntt-examples.json` records ordering and oracle examples for future cross-language comparison. It is not an official NIST vector file.

## Constant-Time Caveat

Passing NTT functional tests does not prove constant-time behavior. Once NTT operates on secret-bearing values, review must cover branches, indexing, memory access, and compiler output in optimized builds.

