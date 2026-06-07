# Rust Derivation: Polynomial Ring

The initial Rust crate includes internal polynomial-ring derivation code at
`rust/pqcore/src/polynomial.rs`.

The Rust implementation derives polynomial arithmetic from `docs/polynomial-ring.md` and the shared
fixture at `fixtures/polynomial-examples.json`.

## Future Type Shape

The Rust code should eventually represent the modulus and degree in the type:

```text
Polynomial<N, Q>
MlKemPolynomial = Polynomial<256, 3329>
MlDsaPolynomial = Polynomial<256, 8380417>
```

The stored coefficients should be field elements, not raw integers.

## Future Stored Representation

Store exactly `N` canonical field elements:

```text
[ModQ<Q>; N]
```

Construction from raw integers should normalize at the boundary. Internal polynomial operations should stay in field-element space.

## Future Red Tests

1. `Polynomial<4, 17>::zero()` has four zero coefficients.
2. Construction normalizes coefficients such as `17`, `-1`, `18`, and `-18`.
3. Addition is coefficient-wise.
4. Subtraction wraps coefficients below zero.
5. In `Z_17[x] / (x^4 + 1)`, `x^3 * x` is `-1`.
6. Multiplication by zero returns zero.
7. Multiplication by one returns the input polynomial.
8. Multiplication distributes over addition for a tiny-ring example.
9. For `MlKemPolynomial`, `x^255 * x` has coefficient zero equal to `3328` and all other coefficients zero.
10. `MlKemPolynomial` and `MlDsaPolynomial` are distinct aliases or types.
11. The shared polynomial fixture parses without introducing a production vector claim.

## Future Fixture Use

The future Rust tests should read or mirror `fixtures/polynomial-examples.json` only as repository-owned derivation examples. Official conformance must wait for NIST ACVP/KAT vectors.

## Constant-Time Note

Rust memory safety does not imply constant-time polynomial arithmetic. Once polynomials carry secret coefficients, indexing, branching, and debug exposure must be reviewed separately.

## Readiness Caveat

Schoolbook polynomial arithmetic is a correctness oracle. It does not implement ML-KEM or ML-DSA by itself, and it is not the final performance path.
