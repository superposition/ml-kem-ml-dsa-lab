# Rust Derivation: Parameter Tables

The initial Rust crate includes internal parameter-table derivation code at
`rust/pqcore/src/params.rs`.

The Rust implementation derives parameter tables from `docs/parameter-tables.md` and the shared
fixture at `fixtures/parameter-sets.json`.

## Future Type Shape

The Rust code should eventually use distinct enums:

```text
MlKemParameterSet = MlKem512 | MlKem768 | MlKem1024
MlDsaParameterSet = MlDsa44 | MlDsa65 | MlDsa87
```

They should not be interchangeable. A future ML-KEM API should not accept an ML-DSA parameter set, and a future ML-DSA API should not accept an ML-KEM parameter set.

## Future ML-KEM Fields

- name
- polynomial degree `n`
- modulus `q`
- module dimension `k`
- `eta1`
- `eta2`
- `du`
- `dv`
- required RBG strength
- security category
- public key bytes
- secret key bytes
- ciphertext bytes
- shared secret bytes

## Future ML-DSA Fields

- name
- polynomial degree `n`
- modulus `q`
- root `zeta`
- dimensions `k` and `l`
- `d`
- `tau`
- `lambda`
- `gamma1`
- `gamma2`
- `eta`
- `beta`
- `omega`
- challenge entropy bits
- security category
- public key bytes
- secret key bytes
- signature bytes

## Future Red Tests

1. Every parameter-set name exactly matches the standardized FIPS name.
2. ML-KEM byte lengths match FIPS 203 Table 3.
3. ML-DSA byte lengths match FIPS 204 Table 2.
4. ML-KEM category values are 1, 3, and 5.
5. ML-DSA category values are 2, 3, and 5.
6. Rust cannot pass an ML-DSA parameter set to an ML-KEM function.
7. The shared fixture can be parsed without using it as an algorithm source of truth.

## Readiness Caveat

Parameter tables are metadata. Correct metadata is required for vector tests, but it does not implement ML-KEM or ML-DSA.
