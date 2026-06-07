# Rust Derivation: Field Elements

There is intentionally no Rust crate for this work yet.

The future Rust implementation should derive field elements from `docs/field-elements.md`, FIPS 203 Table 2, and FIPS 204 Table 1.

## Future Type Shape

The Rust code should eventually use a type that makes the modulus part of the type:

```text
ModQ<Q>
MlKemField = ModQ<3329>
MlDsaField = ModQ<8380417>
```

Raw integers should not be accepted where a reduced field element is required.

## Future Stored Representation

Store the canonical representative in `[0, q)`.

Expose centered representation only as a view:

```text
if value > floor(q / 2) {
    value - q
} else {
    value
}
```

## Future Red Tests

1. `ModQ<3329>(3329).value() == 0`.
2. `ModQ<8380417>(8380417).value() == 0`.
3. `ModQ<3329>(-1).value() == 3328`.
4. `ModQ<8380417>(-1).value() == 8380416`.
5. Addition wraps above the modulus.
6. Subtraction wraps below zero.
7. `(q - 1) * (q - 1) == 1` for `q = 8380417`.
8. Centered representation maps `floor(q / 2) + 1` to a negative value.
9. Deterministic loop tests compare normalization against integer remainder logic for small positive and negative inputs.

## Constant-Time Note

Rust memory safety does not imply constant-time arithmetic. Once the field type is used in secret-bearing paths, branches and memory access patterns must be reviewed separately.

## Readiness Caveat

Field arithmetic is foundational metadata and representation work. It does not implement ML-KEM or ML-DSA.

